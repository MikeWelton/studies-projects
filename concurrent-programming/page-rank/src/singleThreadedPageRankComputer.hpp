#ifndef SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_
#define SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "immutable/network.hpp"
#include "immutable/pageIdAndRank.hpp"
#include "immutable/pageRankComputer.hpp"

class SingleThreadedPageRankComputer : public PageRankComputer {
public:
    SingleThreadedPageRankComputer() {};

    /* HashMap is extremely slow due to usage of linked lists as a buckets for hashes.
     * So we change all maps (except for one needed to find links) to vectors where
     * page is identified not by pageId but by its position number in vector.
     */
    std::vector<PageIdAndRank> computeForNetwork(Network const& network, double alpha, uint32_t iterations, double tolerance) const
    {
        // Auxiliary map to save position in vector for every PageId.
        std::unordered_map<PageId, uint32_t, PageIdHash> pageHashMap;
        std::vector<std::pair<PageId, PageRank>> pagesVector;
        pagesVector.reserve(network.getSize());
        for (auto const& page : network.getPages()) {
            page.generateId(network.getGenerator());
            pageHashMap[page.getId()] = pagesVector.size();
            pagesVector.push_back({ page.getId(), 1.0 / network.getSize() });
        }

        std::vector<uint32_t> numLinks;
        numLinks.reserve(network.getSize());
        for (auto const& page : network.getPages()) {
            numLinks.push_back(page.getLinks().size());
        }

        auto& pages = network.getPages();
        std::vector<uint32_t> danglingNodes;
        for (uint32_t i = 0; i < network.getSize(); ++i) {
            if (pages[i].getLinks().size() == 0) {
                danglingNodes.push_back(i);
            }
        }

        std::vector<std::vector<uint32_t>> edges(pages.size(), std::vector<uint32_t>());
        for (uint32_t i = 0; i < pages.size(); ++i) {
            for (auto link : pages[i].getLinks()) {
                edges[pageHashMap[link]].push_back(i);
            }
        }

        for (uint32_t i = 0; i < iterations; ++i) {
            std::vector<std::pair<PageId, PageRank>> previousPagesVector = pagesVector;

            double dangleSum = 0;
            for (auto danglingNode : danglingNodes) {
                dangleSum += previousPagesVector[danglingNode].second;
            }
            dangleSum = dangleSum * alpha;

            double difference = 0;
            for (uint32_t j = 0; j < pagesVector.size(); ++j) {
                double danglingWeight = 1.0 / network.getSize();
                pagesVector[j].second = dangleSum * danglingWeight + (1.0 - alpha) / network.getSize();

                if (edges[j].size() != 0) {
                    for (auto link : edges[j]) {
                        pagesVector[j].second += alpha * previousPagesVector[link].second / numLinks[link];
                    }
                }
                difference += std::abs(previousPagesVector[j].second - pagesVector[j].second);
            }

            if (difference < tolerance) {
                std::vector<PageIdAndRank> result;
                for (auto iter : pagesVector) {
                    result.push_back(PageIdAndRank(iter.first, iter.second));
                }
                ASSERT(result.size() == network.getSize(), "Invalid result size=" << result.size() << ", for network" << network);

                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);
    }

    /*std::vector<PageIdAndRank> computeForNetwork(Network const& network, double alpha, uint32_t iterations, double tolerance) const {
        std::unordered_map<PageId, PageRank, PageIdHash> pageHashMap;
        for (auto const& page : network.getPages()) {
            page.generateId(network.getGenerator());
            pageHashMap[page.getId()] = 1.0 / network.getSize();
        }

        std::unordered_map<PageId, uint32_t, PageIdHash> numLinks;
        for (auto page : network.getPages()) {
            numLinks[page.getId()] = page.getLinks().size();
        }

        std::unordered_set<PageId, PageIdHash> danglingNodes;
        for (auto page : network.getPages()) {
            if (page.getLinks().size() == 0) {
                danglingNodes.insert(page.getId());
            }
        }

        std::unordered_map<PageId, std::vector<PageId>, PageIdHash> edges;
        for (auto page : network.getPages()) {
            for (auto link : page.getLinks()) {
                edges[link].push_back(page.getId());
            }
        }

        for (uint32_t i = 0; i < iterations; ++i) {
            std::unordered_map<PageId, PageRank, PageIdHash> previousPageHashMap = pageHashMap;

            double dangleSum = 0;
            for (auto danglingNode : danglingNodes) {
                dangleSum += previousPageHashMap[danglingNode];
            }
            dangleSum = dangleSum * alpha;

            double difference = 0;
            for (auto& pageMapElem : pageHashMap) {
                PageId pageId = pageMapElem.first;

                double danglingWeight = 1.0 / network.getSize();
                pageMapElem.second = dangleSum * danglingWeight + (1.0 - alpha) / network.getSize();

                if (edges.count(pageId) > 0) {
                    for (auto link : edges[pageId]) {
                        pageMapElem.second += alpha * previousPageHashMap[link] / numLinks[link];
                    }
                }
                difference += std::abs(previousPageHashMap[pageId] - pageHashMap[pageId]);
            }

            std::vector<PageIdAndRank> result;
            for (auto iter : pageHashMap) {
                result.push_back(PageIdAndRank(iter.first, iter.second));
            }

            ASSERT(result.size() == network.getSize(), "Invalid result size=" << result.size() << ", for network" << network);

            if (difference < tolerance) {
                return result;
            }
        }

        ASSERT(false, "Not able to find result in iterations=" << iterations);
    }*/

    std::string getName() const
    {
        return "SingleThreadedPageRankComputer";
    }
};

#endif /* SRC_SINGLETHREADEDPAGERANKCOMPUTER_HPP_ */
