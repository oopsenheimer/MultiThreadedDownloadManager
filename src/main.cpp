#include <sys/types.h>
#include "DownloaderV2.hpp"

constexpr auto DOWNLOAD_URL = "http://david.choffnes.com/classes/cs5700f22/10MB.log";

int main() {
    DownloaderV2 mydownloader(DOWNLOAD_URL);
    mydownloader.download();
}