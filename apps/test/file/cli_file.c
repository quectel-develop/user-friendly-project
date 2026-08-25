#include "QuectelConfig.h"
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__
#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
#include "ql_file.h"
#include "cli_file.h"
#include "qosa_log.h"

static QL_FILE s_file_stream[10] = {0};
extern void at_print_raw_cmd(const char *type, const char *cmd, size_t size);
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
void PrintfsList(ql_file_info_s *list)
{
    LOG_I("file list:");
    while (list)
    {
        LOG_I("\t%s %d", list->filename, list->filesize);
        list = list->next;
    }
    LOG_I("\n");
}

int cli_file_test(s32_t argc, char *argv[])
{
    if (argc < 2)
    {
        LOG_E("Invalid parameter");
        cli_file_get_help();
        return -1;
    }
    int ret = -1;
    switch(atoi(argv[1]))
    {
        case 0:  // LIST
            ql_file_info_s *list =NULL;
            ret = ql_file_list(at_client_get_first(), argv[2], &list);
            if (0 == ret)
            {
                PrintfsList(list);
                ql_free_file_list(list);
                LOG_I("file list success");
            }
            else
            {
                LOG_E("file list failed");
            }       
            return ret;
        case 1:  // DEL
            ret = ql_remove(at_client_get_first(), argv[2]);
            if (0 == ret)
            {
                LOG_I("file delete success");
            }
            else
            {
                LOG_E("file delete failed");
            }
            return ret;

        case 2:  // FREE
            size_t free_size, total_size;
            LOG_V("name_pattern %s.", argv[2]);
            ret = ql_get_storage_space(at_client_get_first(), argv[2], &free_size, &total_size);
            if (0 == ret)
                LOG_I("query storage success, Free size: %d, Total size: %d", free_size, total_size);
            else
                LOG_E("query storage failed");
            return ret;

        case 3:  // OPEN
        {
            QL_FILE file = ql_fopen(argv[2], atoi(argv[3]));
            if (NULL == file)
            {
                LOG_E("file open failed");
                return -1;
            }
            int i = 0;
            for (i = 0; i < 10; i++)
            {
                if (NULL == s_file_stream[i])
                {
                    s_file_stream[i] = file;
                    break;
                }
            }
            if (i < 10)
                LOG_I("file open success, file handle %d", s_file_stream[i]->fd);
            return 0;
        }
        case 4:  // WRITE
        {
            QL_FILE file = NULL;
            for (int i = 0; i < 10; i++)
            {
                if (s_file_stream[i] != NULL && s_file_stream[i]->fd == atoi(argv[2]))
                {
                    file = s_file_stream[i];
                    break;
                }
            }
            if (file == NULL)
            {
                LOG_E("Invalid file handle");
                break;
            }
            int min_len = (atoi(argv[3]) > strlen(argv[4])) ? strlen(argv[4]) : atoi(argv[3]);
            ret = ql_fwrite(argv[4], 1, min_len, file);
            if (ret > 0)
            {
                LOG_I("file write success, write_size %d", ret);
            }
            else
            {
                LOG_E("file write failed");
            }
            break;
        }
        case 5:  // CLOSE
        {
            QL_FILE file = NULL;
            for (int i = 0; i < 10; i++)
            {
                if (s_file_stream[i] != NULL && s_file_stream[i]->fd == atoi(argv[2]))
                {
                    file = s_file_stream[i];
                    s_file_stream[i] = NULL;
                    break;
                }
            }
            if (file == NULL)
            {
                LOG_E("Invalid file handle");
                break;
            }
            ret = ql_fclose(file);
            if (0 ==ret)
            {
                LOG_I("file close success");
            }
            else
            {
                LOG_E("file close failed");
            }
            return ret;
        }
        case 6:  // READ
        {
            QL_FILE file = NULL;
            for (int i = 0; i < 10; i++)
            {
                if (s_file_stream[i] != NULL && s_file_stream[i]->fd == atoi(argv[2]))
                {
                    file = s_file_stream[i];
                    break;
                }
            }
            if (file == NULL)
            {
                LOG_E("Invalid file handle");
                break;
            }
            char *read_data = (char*)malloc(atoi(argv[3]) + 1);
            if (NULL == read_data)
            {
                LOG_E("read size too large");
                return -1;
            }
            memset(read_data, 0, atoi(argv[3]) + 1);
            int read_len = ql_fread(read_data, 1, atoi(argv[3]), file);
            if (read_len >= 0)
            {
                at_print_raw_cmd("Read data", read_data, read_len);
                FIL fil;
                FRESULT res = f_open(&fil, "0:read.txt", FA_CREATE_ALWAYS | FA_WRITE);
                UINT size = 0;
                if(FR_OK == res)
                {
                    f_write(&fil, read_data, read_len, &size);
                    f_close(&fil);
                }
                LOG_I("file read success, read size %d", read_len);
            }
            else
            {
                LOG_E("file read failed");
            }
            free(read_data);
            return read_len >=0 ? 0 : -1;
        }
        default:
            break;
    }
    return 0;
}

#else
void cli_file_get_help(void)
{
    LOG_W("This function is not supported");
}
int cli_file_test(s32_t argc, char *argv[])
{
    LOG_W("This function is not supported");
}
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_CLI_TEST__ */
