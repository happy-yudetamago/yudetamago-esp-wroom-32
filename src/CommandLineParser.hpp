
#pragma once

class CommandLineParser
{
public:
    CommandLineParser(const char *line) : line(line) {}

public:
    bool Parse() {
        int bufferSize = strlen(line) + 1;
        if (bufferSize > kBufferSize) {
            return false;
        }
        size = bufferSize;
        strncpy(buffer, line, size);
        for (int i=0; i<kBufferSize; i++) {
            switch (buffer[i]) {
            case ' ':  buffer[i] = '\0'; break;
            case '\0': return true;
            }
        }
        return false;
    }

    const char *GetName() const {
        return buffer;
    }

    const char *GetFirstArg() const {
        return NextArg(buffer);
    }

    const char *NextArg(const char *arg) const {
        if (arg < buffer) {
            return 0;
        }
        if ((arg - buffer) >= size) {
            return 0;
        }
        const char *p = arg;
        p = SkipArg(p);
        p = SkipSpace(p);

        if ((p - buffer) >= size) {
            return 0;
        }
        return p;
    }

private:
    enum {
        kBufferSize = 256,
    };
    char buffer[kBufferSize];
    const char *line;
    int size;

private:
    const char *SkipSpace(const char *arg) const {
        int i = arg - buffer;
        for (; i<kBufferSize; i++) {
            if (buffer[i] == '\0') {
                continue;
            }
            return &buffer[i];
        }
        return 0;
    }

    const char *SkipArg(const char *arg) const {
        int i = arg - buffer;
        for (; i<kBufferSize; i++) {
            if (buffer[i] != '\0') {
                continue;
            }
            return &buffer[i];
        }
        return 0;
    }
};
