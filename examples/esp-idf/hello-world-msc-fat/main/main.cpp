#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <driver/sdmmc_defs.h>
#include <driver/sdspi_host.h>
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>
#include <esp_spiffs.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "diskio.h"
#include <fcntl.h>
#include "ff.h"

#include "esp_err.h"
#include "esp_log.h"

#include "flashdisk.hpp"

using namespace esptinyusb;

USBflash disk;

void readfile(const char *path)
{
    char chunk[500] = {};
    char _path[256] = {};
    sprintf(_path, "/fat/%s", path);
    FILE *fd = NULL;
    fd = fopen(_path, "r");
    if (!fd)
    {
        return;
    }

    size_t chunksize = 0;
    do
    {
            puts(chunk);
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, 500, fd);

        /* Keep looping till the whole file is sent */
    } while (chunksize > 0);

    /* Close file after sending complete */
    fclose(fd);
}

int listdir(const char *path)
{
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path);
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dp)))
    {
        puts(entry->d_name);

        readfile(entry->d_name);
    }

    closedir(dp);
    return 0;
}

extern "C" void app_main()
{
    if (!disk.begin())
        printf("failed to begin cdc\n");

    disk.partition("/fat", "ffat");

    listdir("/fat");
}
