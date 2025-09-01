#include <windows.h>
#include <time.h>
#include <ctype.h>
#include "qosa_def.h"
#include "qosa_system.h"
#include "qosa_socket.h"

uint64_t qosa_get_uptime_milliseconds(void)
{
    ULONGLONG uptime = GetTickCount64();
    return uptime;
}

/**
 * 将十六进制字符串转换为字节序列。
 *
 * @param buf 十六进制字符串的输入缓冲区。
 * @param len 十六进制字符串的长度，必须为偶数。
 * @param bufout 转换后的字节序列的输出缓冲区。
 * @return 如果转换成功，返回0；如果输入参数无效或字符串包含非十六进制字符，返回-1。
 */
int hexstr2byte(const char *buf, int len, char *bufout)
{
    int     ret = -1;  // 初始化返回值为-1，表示失败
    int     i = 0;
    uint8_t low;       // 低四位十六进制字符转换的临时变量
    uint8_t high;      // 高四位十六进制字符转换的临时变量

    // 检查输入参数是否有效
    if (NULL == buf || len <= 0 || NULL == bufout)
    {
        return ret;  // 如果参数无效，直接返回-1
    }

    ret = 0;  // 设置返回值为0，表示成功
    // 遍历十六进制字符串，每次处理两个字符
    for (i = 0; i < len; i = i + 2)
    {
        // 处理高四位十六进制字符
        if (((buf[i]) >= '0') && (buf[i] <= '9'))
        {
            high = (uint8_t)(buf[i] - '0');
        }
        else if ((buf[i] >= 'A') && (buf[i] <= 'F'))
        {
            high = (uint8_t)(buf[i] - 'A') + 10;
        }
        else if ((buf[i] >= 'a') && (buf[i] <= 'f'))
        {
            high = (uint8_t)(buf[i] - 'a') + 10;
        }
        else
        {
            ret = -1;  // 如果字符不是有效的十六进制字符，设置返回值为-1
            break;     // 并结束循环
        }

        // 处理低四位十六进制字符
        if (((buf[i + 1]) >= '0') && (buf[i + 1] <= '9'))
        {
            low = (char)(buf[i + 1] - '0');
        }
        else if ((buf[i + 1] >= 'A') && (buf[i + 1] <= 'F'))
        {
            low = (char)(buf[i + 1] - 'A') + 10;
        }
        else if ((buf[i + 1] >= 'a') && (buf[i + 1] <= 'f'))
        {
            low = (uint8_t)(buf[i + 1] - 'a') + 10;
        }
        else
        {
            ret = -1;  // 如果字符不是有效的十六进制字符，设置返回值为-1
            break;     // 并结束循环
        }

        // 将高低四位合并，存入输出缓冲区
        bufout[i / 2] = (char)((high << 4) | (low & 0x0F));
    }
    return ret;  // 返回转换结果
}

/**
 * 将字节数据转换为十六进制字符串。
 *
 * @param bufin 输入的字节数据指针。
 * @param in_len 输入字节数据的长度。
 * @param bufout 输出的十六进制字符串指针。
 * @param out_len 输出十六进制字符串的最大长度。
 * @return 如果转换成功，返回0；如果参数错误，返回-1。
 *
 * @note 函数确保输出的十六进制字符串长度不超过out_len的一半。
 */
int byte2hexstr(const char *bufin, int in_len, char *bufout, int out_len)
{
    int     i = 0;
    int     len = 0;
    uint8_t tmp_l = 0x0;
    uint8_t tmp_h = 0;

    // 检查输入和输出参数是否有效
    if ((NULL == bufin) || (in_len <= 0) || (NULL == bufout) || (out_len <= 0))
    {
        return -1;
    }

    // 计算实际可以转换的字节数量，确保不超过输出缓冲区的一半
    len = (in_len > out_len / 2) ? (out_len / 2) : in_len;
    len -= 1;

    // 遍历每个字节，转换为两位十六进制字符
    for (i = 0; i < len; i++)
    {
        // 提取字节的高4位和低4位
        tmp_h = (bufin[i] >> 4) & 0X0F;
        tmp_l = bufin[i] & 0x0F;

        // 将高4位和低4位转换为对应的十六进制字符
        bufout[2 * i] = (tmp_h > 9) ? (tmp_h - 10 + 'a') : (tmp_h + '0');
        bufout[2 * i + 1] = (tmp_l > 9) ? (tmp_l - 10 + 'a') : (tmp_l + '0');
    }

    // 在字符串末尾添加终止符，标记字符串结束
    bufout[2 * len] = '\0';

    //转换成功，返回0
    return 0;
}


/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii representation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int osal_ip4addr_aton(const char *cp, ip4_addr_t *addr)
{
    u32_t val;
    uint8_t base;
    char c;
    u32_t parts[4];
    u32_t *pp = parts;

    c = *cp;
    for (;;)
    {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
        {
            return 0;
        }
        val = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c = *++cp;
            }
            else
            {
                base = 8;
            }
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (u32_t) (c - '0');
                c = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (u32_t) (c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
            {
                break;
            }
        }
        if (c == '.')
        {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
            {
                return 0;
            }
            *pp++ = val;
            c = *++cp;
        }
        else
        {
            break;
        }
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
    {
        return 0;
    }
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1)
    {

    case 0:
        return 0; /* initial nondigit */

    case 1: /* a -- 32 bits */
        break;

    case 2: /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
        {
            return 0;
        }
        if (parts[0] > 0xff)
        {
            return 0;
        }
        val |= parts[0] << 24;
        break;

    case 3: /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    default:
        QOSA_ASSERT(0);
        break;
    }
    if (addr)
    {
        ip4_addr_set_u32(addr, htonl(val));
    }
    return 1;
}

/**
 * Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param addr ip address in network order to convert
 * @param buf target buffer where the string is stored
 * @param buflen length of buf
 * @return either pointer to buf which now holds the ASCII
 *         representation of addr or NULL if buf was too small
 */
char *ipaddr_ntoa_r(const ip_addr_t *addr, char *buf, int buflen)
{
  u32_t saddr;
  char inv[3];
  char *rp;
  u8_t *ap;
  u8_t rem;
  u8_t n;
  u8_t i;
  int len = 0;

  saddr = ip4_addr_get_u32(addr);

  rp = buf;
  ap = (u8_t *)&saddr;
  for(n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (u8_t)10;
      *ap /= (u8_t)10;
      inv[i++] = '0' + rem;
    } while(*ap);
    while(i--) {
      if (len++ >= buflen) {
        return NULL;
      }
      *rp++ = inv[i];
    }
    if (len++ >= buflen) {
      return NULL;
    }
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return buf;
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         represenation of addr
 */
char *
ipaddr_ntoa(const ip_addr_t *addr)
{
  static char str[16];
  return ipaddr_ntoa_r(addr, str, 16);
}

/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
u32_t ipaddr_addr(const char *cp)
{
  ip_addr_t val;

  if (inet_aton(cp, &val)) {
    return ip4_addr_get_u32(&val);
  }
  return (IPADDR_NONE);
}
