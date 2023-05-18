#include "Logging.h"
#include "FileUtil.h"

FileUtil::FileUtil(std::string &fileName)
  : fp_(::fopen(fileName.c_str(), "ae")),
      writtenBytes_(0)
{
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileUtil::~FileUtil()
{
    ::fclose(fp_);
}

void FileUtil::append(const char *data, size_t len) {
    size_t written = 0;

    while(written != len) {
      size_t remain = len - written;
      size_t n = write(data + written, remain);
      if(n != remain) {
        int err = ferror(fp_);
        if(err) {
          fprintf(stderr, "FileUtil::append() failed%s \n", getErrnoMsg(errno));
        }
      }
      written += n;
    }

    writtenBytes_ += written;
}

void FileUtil::flush() {
    ::fflush(fp_);
}

size_t FileUtil::write(const char *data, size_t len) {
    return ::fwrite_unlocked(data, 1, len, fp_);
}



