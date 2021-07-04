#ifndef SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_

#include <atomic>
#include <mutex>
#include <thread>

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../tests/lib/performanceTimer.hpp"
#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"

class MultiThreadedPageRankComputer : public PageRankComputer {
public:
    MultiThreadedPageRankComputer(uint32_t numThreadsArg)
        : numThreads(numThreadsArg)
        , mutex()
        , locks()
        , pagesVector()
        , numLinks()
        , danglingNodes()
        , edges()
        , previousPagesVector()
        , difference()
    {
        threads.reserve(numThreadsArg);
    };

    std::vector<PageIdAndRank> computeForNetwork(Network const& network, double alpha, uint32_t iterations, double tolerance) const
    {
        std::vector<pair_uint32_t> ranges = prepareRanges(network.getSize());
        initMembers(network, ranges);
        std::vector<pair_uint32_t> dangleRanges = prepareRanges(danglingNodes.size());

        double beta = (1.0 - alpha) / network.getSize();
        for (uint32_t i = 0; i < iterations; ++i) {
            previousPagesVector = pagesVector;
            difference = 0.0;

            double dangleSum = danglingNodesSum(dangleRanges);
            dangleSum *= alpha;

            PageRank partialPageRank = dangleSum / network.getSize() + beta;
            for (auto& range : ranges) {
                auto lambda = [partialPageRank, alpha, &range, this]() {
                    calculatePageRank(partialPageRank, alpha, range);
                };
                threads.emplace_back(lambda);
            }

            for (auto& thread : threads) {
                thread.join();
            }
            threads.clear();

            if (difference < tolerance) {
                std::vector<PageIdAndRank> result;
                result.reserve(pagesVector.size());

                for (auto const& pair : pagesVector) {
                    result.push_back(PageIdAndRank(pair.first, pair.second));
                }

                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);
    }

    std::string getName() const
    {
        return "MultiThreadedPageRankComputer[" + std::to_string(this->numThreads) + "]";
    }

private:
    /* In vectors below, every page is not identified by its id but
     * by its position number in pagesVector. */
    uint32_t numThreads;
    mutable std::mutex mutex; // Main mutex.
    mutable std::vector<std::mutex> locks; // One mutex for every page.
    mutable std::vector<std::thread> threads;
    mutable std::vector<std::pair<PageId, PageRank>> pagesVector;
    mutable std::vector<uint32_t> numLinks;
    mutable std::vector<uint32_t> danglingNodes;
    mutable std::vector<std::vector<uint32_t>> edges;
    mutable std::vector<std::pair<PageId, PageRank>> previousPagesVector;
    mutable double difference;

    using pair_uint32_t = std::pair<uint32_t, uint32_t>;

    /* Clears all member containers and initializes them with new data. */
    void initMembers(Network const& network, std::vector<pair_uint32_t>& ranges) const
    {
        locks = std::vector<std::mutex>(network.getSize());
        pagesVector.clear();
        numLinks.clear();
        danglingNodes.clear();
        edges.clear();
        previousPagesVector.clear();

        // Auxiliary map to save position in vector for every PageId.
        std::unordered_map<PageId, uint32_t, PageIdHash> pageHashMap;
        pageHashMap.reserve(network.getSize());
        pagesVector.reserve(network.getSize());

        generateIds(network, ranges);

        for (auto const& page : network.getPages()) {
            pageHashMap[page.getId()] = pagesVector.size();
            pagesVector.push_back({ page.getId(), 1.0 / network.getSize() });
        }

        numLinks.reserve(network.getSize());
        for (auto const& page : network.getPages()) {
            numLinks.push_back(page.getLinks().size());
        }

        std::vector<Page> const& pages = network.getPages();
        for (uint32_t i = 0; i < network.getSize(); ++i) {
            if (pages[i].getLinks().size() == 0) {
                danglingNodes.push_back(i);
            }
        }

        saveLinks(pageHashMap, pages, ranges);
    }

    /* Runs threads to generate ids concurrently. Every thread gets its own
     * operational range of positions in pagesVector.*/
    void generateIds(Network const& network, std::vector<pair_uint32_t>& ranges) const
    {
        IdGenerator& generator = (IdGenerator&)(network.getGenerator());
        auto& pages = network.getPages();
        for (auto& range : ranges) {
            auto lambda = [&range, &generator, &pages, this]() {
                generateIdsConcurrent(range, generator, pages);
            };
            threads.emplace_back(lambda);
        }
        for (auto& thread : threads) {
            thread.join();
        }
        threads.clear();
    }

    /* Generates ids for pages which are stored in pagesVector, starting from position
     * range.first and ending on range.second. */
    void generateIdsConcurrent(pair_uint32_t& range, IdGenerator const& generator, std::vector<Page> const& pages) const
    {
        for (uint32_t i = range.first; i <= range.second; ++i) {
            pages[i].generateId(generator);
        }
    }

    /* Calculates number of dangling nodes. Every thread gets reference to
     * variable with dangleSum.*/
    double danglingNodesSum(std::vector<pair_uint32_t>& ranges) const
    {
        double dangleSum = 0.0;
        for (auto& range : ranges) {
            auto lambda = [&range, &dangleSum, this]() { countDangleSum(range, dangleSum); };
            threads.emplace_back(lambda);
        }

        for (auto& thread : threads) {
            thread.join();
        }
        threads.clear();

        return dangleSum;
    }

    /* Counts number of dangling nodes in given range of pages. Result is put in shared
     * dangleSum variable. */
    void countDangleSum(pair_uint32_t& range, double& dangleSum) const
    {
        double dangleSumLocal = 0.0;
        for (uint32_t i = range.first; i <= range.second; ++i) {
            dangleSumLocal += pagesVector[danglingNodes[i]].second;
        }

        mutex.lock();
        dangleSum += dangleSumLocal;
        mutex.unlock();
    }

    /* Saves links leading to pages. */
    void saveLinks(std::unordered_map<PageId, uint32_t, PageIdHash>& pageHashMap, std::vector<Page> const& pages, std::vector<pair_uint32_t>& ranges) const
    {
        edges = std::vector<std::vector<uint32_t>>(pages.size(), std::vector<uint32_t>());
        for (auto& range : ranges) {
            auto lambda = [&pageHashMap, &pages, &range, this]() {
                saveLinksConcurrent(pageHashMap, pages, range);
            };
            threads.emplace_back(lambda);
        }

        for (auto& thread : threads) {
            thread.join();
        }
        threads.clear();
    }

    /* Browses links that are present on pages from given range. When adding new link
     * leading to some site, function locks on mutex corresponding to that site. Uses
     * HashMap to find position of site. (That operation is basically only reason
     * why that HashMap exists.) */
    void saveLinksConcurrent(std::unordered_map<PageId, uint32_t, PageIdHash>& pageHashMap, std::vector<Page> const& pages, pair_uint32_t& range) const
    {
        for (uint32_t i = range.first; i <= range.second; ++i) {
            for (auto& link : pages[i].getLinks()) {
                uint32_t k = pageHashMap[link];
                locks[k].lock();
                edges[k].push_back(i);
                locks[k].unlock();
            }
        }
    }

    /* Returns ranges of pages to operate on for every thread. Position i in vector contains
     * range for i-th thread. In pair: first is the first page in range and second is last
     * page in range. Pages are identified by their position in vector of size given
     * as parameter.*/
    std::vector<pair_uint32_t> prepareRanges(uint32_t size) const
    {
        if (size == 0) {
            return std::vector<pair_uint32_t>(numThreads, { 1, 0 });
        }

        std::vector<pair_uint32_t> ranges(numThreads);
        uint32_t start = 0;

        for (uint32_t i = 0; i < numThreads; ++i) {
            uint32_t remainder = size % numThreads;
            uint32_t numPages = size / numThreads + (i < remainder ? 1 : 0);

            ranges[i] = { start, start + numPages - 1 };
            start += numPages;
        }

        return ranges;
    }

    /* Calculates pageRank for pages from given range of positions.*/
    void calculatePageRank(PageRank partialPageRank, double alpha, pair_uint32_t& range) const
    {
        double differenceLocal = 0.0;
        for (uint32_t i = range.first; i <= range.second; ++i) {
            pagesVector[i].second = partialPageRank;

            if (edges[i].size() != 0) {
                for (auto link : edges[i]) {
                    pagesVector[i].second += alpha * previousPagesVector[link].second / numLinks[link];
                }
                differenceLocal += std::abs(previousPagesVector[i].second - pagesVector[i].second);
            }
        }

        mutex.lock();
        difference += differenceLocal;
        mutex.unlock();
    }
};

#endif /* SRC_MULTITHREADEDPAGERANKCOMPUTER_HPP_ */
