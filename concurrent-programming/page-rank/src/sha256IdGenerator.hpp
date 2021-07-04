#ifndef SRC_SHA256IDGENERATOR_HPP_
#define SRC_SHA256IDGENERATOR_HPP_

#include "immutable/idGenerator.hpp"
#include "immutable/pageId.hpp"
#include <cassert>

#define BUF_SIZE 65 // 64 is length of sha256 hash + 1 for \0 in the end

class Sha256IdGenerator : public IdGenerator {
public:
    virtual PageId generateId(std::string const& content) const
    {
        std::string command = "echo -n \"" + content + "\" | sha256sum";
        const char* cmd = command.c_str();
        char buffer[BUF_SIZE];
        FILE* fpipe;
        if ((fpipe = (FILE*)popen(cmd, "r")) == 0)
            ASSERT(false, "Popen error");

        if (fread(buffer, 1, BUF_SIZE, fpipe) != BUF_SIZE)
            ASSERT(false, "Read error\n");

        pclose(fpipe);

        buffer[64] = '\0';
        std::string ret = buffer;

        return ret;
    }
};

#endif /* SRC_SHA256IDGENERATOR_HPP_ */
