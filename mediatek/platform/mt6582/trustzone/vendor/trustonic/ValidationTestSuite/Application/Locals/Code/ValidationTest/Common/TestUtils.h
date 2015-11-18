/**
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */
#ifndef _TESTUTILS_H_
#define _TESTUTILS_H_

#include <sys/types.h>
#include <sys/time.h>
#include "mc_linux.h"
#include "MobiCoreDriverApi.h"

namespace TestUtils {

namespace Random {

const uint8_t DEFAULT_BYTE_PATTERN = 0x5A;

// Option flags for pattern generation
enum opPatternGeneration_t {
    OP_NONE    = 0,
    OP_IGN_00  = 1,
    OP_IGN_FF  = 2,
    OP_IGN_DUP = 4
};

uint32_t getPattern(uint8_t *pattern, uint32_t patternLen, uint32_t op);
uint8_t getByte(uint32_t op = OP_IGN_00 | OP_IGN_FF);

} // namespace Random



namespace FileOperations {


	uint32_t dummy(uint32_t in);

	uint32_t writeDataInFile(const char *, uint8_t*, uint32_t);
	uint32_t writeStringInFile(const char *, uint8_t*);
	uint32_t writeHexDataInFile(const char *, uint8_t*, uint32_t);

} // namespace FileOperations


namespace System {

	void freePageCache();
	void printMemInfo();

} // namespace System

namespace Time {

class Stopwatch
{
  public:
    Stopwatch();
   ~Stopwatch();

    void start();
    int getElapsedMilliseconds();
    int getElapsedMillisecondsAndRestart();

  private:
    struct timeval reference;
};

} // namespace Time

namespace BypassDaemon{

#define SEND_TO_DAEMON(SOCKET, COMMAND, ...) \
    do { \
        struct COMMAND ##_struct x = { \
            COMMAND, \
            __VA_ARGS__ \
        }; \
        int len = sizeof(x); \
        int ret = send((SOCKET), &x, len, 0); \
        CHECK((ret) == (len));\
    }while(0)

#define RECV_FROM_DAEMON(SOCKET, RSP_STRUCT) \
    { \
        int expectedLen = sizeof(*(RSP_STRUCT)); \
        int rlen = recv(SOCKET, \
                (RSP_STRUCT), \
                expectedLen, MSG_WAITALL); \
        rlen < 0 ?  perror("recv") : (void)0;\
        CHECK(rlen == expectedLen); \
    }
    
    #define PIPE_WRITE(fd,x) CHECK( sizeof(x) == write(fd, &(x), sizeof(x)))
    #define PIPE_READ(fd,x) CHECK( sizeof(x) == read(fd, &(x), sizeof(x)))

    #define TO_B(x) PIPE_WRITE(pipefd[0][1], x)
    #define FROM_A(x) PIPE_READ(pipefd[0][0], x)

    #define TO_A(x) PIPE_WRITE(pipefd[1][1], x)
    #define FROM_B(x) PIPE_READ(pipefd[1][0], x)    


    int customOpenDevice();
    int customConnectToDaemon(uint32_t deviceId);
    uint32_t customAllocateMap(::mc_ioctl_map *alloc, int fd);
	mcResult_t customOpenSession(int devfd,
			int sockfd,
			struct mc_ioctl_map *alloc,
			uint32_t *sessionId,
			bool allocp=true);
	mcResult_t customCloseSession(int sockfd, uint32_t sessionId);
	mcResult_t customMcMap(int devfd, int sockfd, uint32_t sessionId, struct mc_ioctl_map *alloc);
	void customMcNotify(int sockfd, uint32_t sessionId);
	mcResult_t customMcWaitNotification(int sockfd, uint32_t sessionId);
	mcResult_t customMcFree(int devfd, uint32_t handle);

} // namespace BypassDaemon

namespace MultiProcess {
	void init_pipes(int pipefd[][2], int count);
	void close_pipes(int pipefd[][2], int count);

	void parentWaitOrKillChildren(
			const pid_t *pid,
			int nPid,
			unsigned int timeout,
			int childpipe[2],
			const bool ignoreChildStatus[] = NULL);
}

namespace DaemonControl {
    void getDaemonPid(char * pid, uint32_t size);
    bool isDaemonAlive(void);
} //namespace DaemonControl


}// namespace TestUtils



/* CHECK macro that should be used in children */
#define CHILD_CHECK( value ) do{ bool b=!!(value); if(!b) { LOG_E( #value ); exit(1); } } while(0)
#define CHILD_CHECK_DRVAPI_RETURNS_OK(x) CHILD_CHECK(MC_DRV_OK == (x))

#endif // _TESTUTILS_H_

