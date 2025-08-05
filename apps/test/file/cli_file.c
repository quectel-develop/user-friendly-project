#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#include "ql_fs.h"
#include "cli_file.h"
#include "qosa_log.h"

char read_data[30 + 1];

void cli_file_get_help(void)
{
    LOG_I("  0.   query files");
    LOG_I("     example: file 0 \"*\"");
    LOG_I("  1.   del files");
    LOG_I("     example: file 1 \"123.txt\"");
    LOG_I("  2.   query free");
    LOG_I("     example: file 2 \"UFS\"");
    LOG_I("  3.   open files");
    LOG_I("     example: file 3 \"text\" 0");
    LOG_I("     example: file 3 filename  mode");
    LOG_I("     mode    Integer type. The open mode of the file");
    LOG_I("     0 If the file does not exist, it is created. If the file exists, it is opened directly. In any case, the file can be read and written.");
    LOG_I("     1 If the file does not exist, it is created. If the file exists, it is overwritten. In any case, the file can be read and written.");
    LOG_I("     2 If the file exists, it is opened directly and is read only. If the file does not exist,an error is returned.");
    LOG_I("     3 If the file does not exist, it is created. If the file exists, write data to the file. In any case, the file can be read and written.");
    LOG_I("  4.   wirte files");
    LOG_I("     example: file 4 1 5 12345");
    LOG_I("     example: file 4 filehandle length DATA");
    LOG_I("     <filehandle> Integer type. The handle of the file to be operated.");
    LOG_I("     <length>     Integer type. The length of the file to be written.");
    LOG_I("     <DATA>       WIRTE DATA");
    LOG_I("  5.   close files");
    LOG_I("     example: file 5 1");
    LOG_I("     example: file 5 filehandle ");
    LOG_I("  6.   read files");
    LOG_I("     example: file 6 1 5");
    LOG_I("     example: file 6 filehandle length");
}

// Function to print a list of files with their details
void PrintfsList(File_Moudle_Info *fileList, int fileCount)
{
    for (int i = 0; i < fileCount; i++)
    {
        // Print the filename, size in kilobytes, and date for each file.
        LOG_V("Filename: %s       , B: %ld      \n",
              fileList[i].filename,
              fileList[i].filesize);
    }
}

int cli_file_test(s32_t argc, char *argv[])
{
    fs_test_config config;
    config.fs_type = atoi(argv[1]);

    switch(config.fs_type)
    {
        case 0:  // LIST
            strcpy(config.name_pattern, argv[2]);
            File_Moudle_Info fileList[32];
            int fileCount = ql_module_list_get(config.name_pattern, fileList, sizeof(fileList) / sizeof(fileList[0]), 0);
            if (fileCount >= 0)
            {
                PrintfsList(fileList, fileCount);
            }
            break;

        case 1:  // DEL
            strcpy(config.name_pattern, argv[2]);
            ql_file_del(config.name_pattern);
            break;

        case 2:  // FREE
            strcpy(config.name_pattern, argv[2]);
            s32_t free_size, total_size;
            LOG_V("name_pattern %s.\n", config.name_pattern);
            if (ql_fs_get_free(config.name_pattern, &free_size, &total_size) == 0)
                LOG_V("Free size: %d, Total size: %d\n", free_size, total_size);
            else
                LOG_V("Failed to get file system info.\n");
            break;

        case 3:  // OPEN
            strcpy(config.name_pattern, argv[2]);
            config.open_mode = atoi(argv[3]);
            u16_t file_handle;
            file_handle = ql_fs_open(config.name_pattern, config.open_mode);
            LOG_V("file_handle %d\n", file_handle);
            break;

        case 4:  // WRITE
            config.file_handle = atoi(argv[2]);
            config.write_read_size = atoi(argv[3]);
            ql_fs_write(config.file_handle, config.write_read_size, config.write_buffer);
            break;

        case 5:  // CLOSE
            config.file_handle = atoi(argv[2]);
            ql_fs_close(config.file_handle);
            break;

        case 6:  // READ
            config.file_handle = atoi(argv[2]);
            config.write_read_size = atoi(argv[3]);
            u16_t read_len = ql_fs_read(config.file_handle, config.write_read_size, read_data);
            if (read_len > 0)
                LOG_V("Read data (%d bytes): %s\n", read_len, read_data);
            else
                LOG_V("Failed to read data.\n");
            break;

        default:  break;
    }
}

#else
void cli_file_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_file_test(int argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
