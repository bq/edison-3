/**
 * <!-- Copyright Giesecke & Devrient GmbH 2010-2012 -->
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "TestUtils.h"

#define LOG_TAG "TestUtils"
#include <log.h>

#include "Common/MobiCoreDriverCmd.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "CppUTest/TestHarness.h"
#include "TrustletForwarder.h"

namespace TestUtils {

namespace Random {

uint32_t getPattern(uint8_t *pattern, uint32_t patternLen, uint32_t op)
{
    uint32_t rdCount = 0;

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd != -1) {
        uint32_t ignCount = 0;
        while (rdCount < patternLen && read(fd, &pattern[rdCount], 1) == 1) {
            if ((op & OP_IGN_00 && pattern[rdCount] == 0) ||
                (op & OP_IGN_FF && pattern[rdCount] == 0xFF) ||
                (rdCount && op & OP_IGN_DUP && pattern[rdCount] == pattern[rdCount-1])) {
                if (++ignCount > 16) break; // Device's joking ;)
                else continue;
            }

            rdCount++;
            ignCount = 0;
        }
        close(fd);
    }

    return rdCount;
}

uint8_t getByte(uint32_t op)
{
    uint8_t byte;

    if (getPattern(&byte, 1, op) != 1) {
        byte = DEFAULT_BYTE_PATTERN;
        LOG_W("Pattern generation failed, using default pattern");
    }

    return byte;
}

} // namespace Random


namespace System {

void freePageCache()
{
    // Free page cache only, don't call sync
    system("echo 1 > /proc/sys/vm/drop_caches");
}

void printMemInfo()
{
    struct system_info {
        unsigned long totalram;  /* Total usable main memory size */
        unsigned long freeram;   /* Available memory size */
    } si;

    FILE *infile = fopen("/proc/meminfo", "r");
    if (infile == NULL) {
        perror("Error opening /proc/meminfo");
        return;
    }

    while (!feof(infile)) {
        char buffer[80];
        if (fgets(buffer, sizeof(buffer), infile)) {
            const char *field = strtok(buffer, " ");
            const char *valueS = strtok(NULL, " ");
            if (field == NULL)
                continue;
            if (valueS == NULL)
                continue;
            unsigned long long value;
            sscanf(valueS, "%lli", &value);

            if (strcmp(field, "MemTotal:") == 0)
                si.totalram = value;
            else if (strcmp(field, "MemFree:") == 0)
                si.freeram = value;
        }
    }
    printf("meminfo free / total [kB]: %ld / %ld\n", si.freeram, si.totalram);
}

} // namespace System


namespace Time {

Stopwatch::Stopwatch()
{
    reference.tv_sec = 0;
    reference.tv_usec = 0;
}

Stopwatch::~Stopwatch()
{
}

void Stopwatch::start()
{
    gettimeofday(&reference, NULL);
}

int Stopwatch::getElapsedMilliseconds()
{
    struct timeval current;
    gettimeofday(&current, NULL);

    if (current.tv_usec & reference.tv_usec)
    {
        current.tv_sec--;
        current.tv_usec += 1000000;
    }

    return ((current.tv_sec  - reference.tv_sec)  * 1000 +
            (current.tv_usec - reference.tv_usec) / 1000);
}

int Stopwatch::getElapsedMillisecondsAndRestart()
{
    int elapsedMs = getElapsedMilliseconds();
    start();
    return elapsedMs;
}

} // namespace Time

namespace FileOperations {

	uint32_t writeDataInFile(const char * fileName, uint8_t* data, uint32_t len) {
		FILE * fp = fopen(fileName, "a");

		if (fp == 0) {
			printf ("File %s could not be opened", fileName);
			return -1;
		}

		for(uint32_t i = 0; i < len; i++) {
			fprintf(fp, "%c", (char)*(data +i));
		}
		fprintf(fp, "\n\r");
		fclose(fp);
		return 1;
	}

	uint32_t writeStringInFile(const char * fileName, uint8_t* str) {
		FILE * fp = fopen(fileName, "a");
		if (fp == 0) {
			printf ("File %s could not be opened", fileName);
			return -1;
		}

		fprintf(fp, "%s", str);
		fprintf(fp, "\n\r");
		fclose(fp);
		return 1;
	}

	uint32_t writeHexDataInFile(const char * fileName, uint8_t* data, uint32_t len) {
		FILE * fp = fopen(fileName, "a");

		if (fp == 0) {
			printf ("File %s could not be opened", fileName);
			return -1;
		}

		for(uint32_t i = 0; i < len; i++) {
			fprintf(fp, " 0x%x,", *(data +i));
		}
		fprintf(fp, "\n\r");
		fclose(fp);
		return 1;
	}

	uint32_t dummy(uint32_t in){
		//TODO
	}

} // namespace FileOperations


namespace BypassDaemon{


/*
 * Invoke daemon's openDevice function, bypassing the ClientLib
 */
int customOpenDevice()
{
    return open("/dev/" MC_USER_DEVNODE, O_RDWR);
}


/*
 * Invoke daemon's openDevice function, bypassing the ClientLib
 */
int customConnectToDaemon(uint32_t deviceId)
{
    static const char *socketName = SOCK_PATH;
    struct sockaddr_un remote;
    int sockfd;
    mcResult_t mcResult,x;

    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, socketName, sizeof(remote.sun_path) - 1);

    CHECK(0 <= (sockfd = socket(AF_UNIX, SOCK_STREAM, 0) ));
    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    // The Daemon socket is in the Abstract Domain(LINUX ONLY!)
    remote.sun_path[0] = 0;
    CHECK( 0 <= ::connect(sockfd, (struct sockaddr *) &remote, len) );

    // Forward device open to the daemon and read result
    SEND_TO_DAEMON(sockfd, MC_DRV_CMD_OPEN_DEVICE, deviceId);
    RECV_FROM_DAEMON(sockfd, &mcResult);
    CHECK_DRVAPI_RETURNS_OK( mcResult );

    return sockfd;
}    


/*
 * Allocate WSM and map it in process user address space, using direct call
 * to the driver
 */
uint32_t customAllocateMap(::mc_ioctl_map* alloc, int fd)
{
    int ret = 0;

    ret = ::ioctl(fd, MC_IO_MAP_WSM, alloc);
    if (ret != 0) {
        printf("ioctl MC_IO_MAP_WSM");
        exit(1);
    }

    void *virtAddr = ::mmap(0, alloc->len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, alloc->phys_addr);
    if (MAP_FAILED == virtAddr)
    {
        printf("mmap() failed with errno: %d", errno);
        exit(1);
    }
    alloc->addr = (long unsigned int)virtAddr;

    //printf("mmap() virt addr = 0x%08p\n", virtAddr);
    //printf(" mapped to %p, handle=%d, phys=%p\n", virtAddr, alloc->handle, (alloc->phys_addr));
    return alloc->handle;
}

mcResult_t customOpenSession(int devfd,
		int sockfd,
		struct mc_ioctl_map *alloc,
		uint32_t *sessionId,
		bool allocp)
{
	mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;
	uint32_t len = 4096;

	if (allocp) {
		alloc->len = 4096;
		customAllocateMap(alloc, devfd);
	}

	SEND_TO_DAEMON(sockfd, MC_DRV_CMD_OPEN_SESSION,
			0,
			TL_TEST_UUID,
			(uint32_t)0,
			(uint32_t)alloc->handle,
			len);

	// Read command response
	RECV_FROM_DAEMON(sockfd, &mcResult);
	CHECK(MC_DRV_OK == mcResult);

	// read payload
	mcDrvRspOpenSession_t rspOpenSession;
	memset(&rspOpenSession, 0xCC, sizeof(rspOpenSession));

	/* get response payload */
	RECV_FROM_DAEMON(sockfd, &rspOpenSession.payload);

	// Register session with handle
	*sessionId = rspOpenSession.payload.sessionId;

	return mcResult;
}

mcResult_t customCloseSession(int sockfd, uint32_t sessionId)
{
	//try to close the session
	mcResult_t mcResult;
	SEND_TO_DAEMON(sockfd, MC_DRV_CMD_CLOSE_SESSION, sessionId);
	RECV_FROM_DAEMON(sockfd, &mcResult);
	return mcResult;
}

/*
 * Reimplementation of mcMap
 */
mcResult_t customMcMap(int devfd, int sockfd, uint32_t sessionId, struct mc_ioctl_map *alloc)
{
	mcResult_t mcResult = MC_DRV_ERR_UNKNOWN;

	int len;
	len = alloc->len = 4096;
	int *p = (int*) malloc(len);
	CHECK (p);

	struct mc_ioctl_reg_wsm params = {
		(uint32_t) p,
		len,
		0
	};

	customAllocateMap(alloc, devfd);
	int ret = ioctl(devfd, MC_IO_REG_WSM, &params);
	CHECK(0 == ret);

	SEND_TO_DAEMON(sockfd, MC_DRV_CMD_MAP_BULK_BUF,
			sessionId,
			params.handle, 
			0,
			(uint32_t)p & 0xFF,
			len);

	// Read command response
	RECV_FROM_DAEMON(sockfd, &mcResult);
	if (MC_DRV_OK != mcResult) {
		return mcResult;
	}

	// init the response payload
	mcDrvRspMapBulkMem_t rspMap;
	memset(&rspMap, 0xCC, sizeof(rspMap));

	/* get response payload */
	RECV_FROM_DAEMON(sockfd, &rspMap.payload);

	// Register session with handle
	printf("Mapped bulk buffer, sessionId = %08x, virtAddr = %08x\n",
			rspMap.payload.sessionId,
			rspMap.payload.secureVirtualAdr);

	return mcResult;
}

mcResult_t customMcFree(int devfd, uint32_t handle)
{
	if (0 == ioctl(devfd, MC_IO_FREE, handle)) {
		return MC_DRV_OK;
	}
	else {
		return MC_DRV_ERR_UNKNOWN;
	}
}

void customMcNotify(int sockfd, uint32_t sessionId)
{
	SEND_TO_DAEMON(sockfd, MC_DRV_CMD_NOTIFY, sessionId);
}

/** Notification data structure. */
typedef struct {
    uint32_t sessionId; /**< Session ID. */
    int32_t payload; /**< Additional notification information. */
} notification_t;

mcResult_t customMcWaitNotification(int sockfd, uint32_t sessionId, int32_t timeout)
{
	mcResult_t mcResult = MC_DRV_OK;
	notification_t notification;
	RECV_FROM_DAEMON(sockfd, &notification);
	if (notification.payload != 0) {
		// Session end point died -> store exit code
		mcResult = MC_DRV_INFO_NOTIFICATION;
	}
	return mcResult;
}

} // namespace BypassDaemon

namespace MultiProcess {

	void init_pipes(int pipefd[][2], int count)
	{
		for (int i=0; i<count; i++) {
			CHECK(0 == pipe(pipefd[i]));
		}
	}

	void close_pipes(int pipefd[][2], int count)
	{
		for (int i=0; i<count; i++) {
			close(pipefd[i][0]);
			close(pipefd[i][1]);
		}
	}

	void parentWaitOrKillChildren(
			const pid_t *pid,
			int nPid,
			unsigned int timeout,
			int childpipe[2],
			const bool ignoreChildStatus[])
	{
		fd_set rfds;
		int ret;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(childpipe[0], &rfds);

		/* wait for at most 2 seconds*/
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		close(childpipe[1]);
		// The child are not supposed to write in this pipe. So select should
		// return only on timeout or when all the children have terminated
		ret = select(childpipe[0] + 1, &rfds, NULL, NULL, &tv);

		// kill all the child in case any of them is still alive
		for(int i=0; i<nPid; i++) {
			kill(pid[i], SIGKILL);
		}

		int status;
		for(int i=0; i<nPid; i++) {
			// get the exit status of the childs
			CHECK( waitpid(pid[i], &status,  0) );
			// Ensure the childs exited correctly
			if(NULL==ignoreChildStatus || !ignoreChildStatus[i]) {
				CHECK( WIFEXITED(status) && 0 == WEXITSTATUS(status) );
			}
		}

	}
} // namespace MultiProcess


namespace DaemonControl{
    /*
     * Daemon process id parser. returns the id by reference
     * Note: For now pid is  up  to 6 digits
     */
    void getDaemonPid(char * pid, uint32_t size) 
    {
    	FILE * pipe_reader;
    	char * cmdResult = (char *) malloc (500 * sizeof(char));
    	char * parsed;
    	char delimiters[] = {' ', '\n', '\r'};

    	// To read what command returns including the data
    	pipe_reader = popen("ps -C mcDriverDaemon", "r");
    	fread(cmdResult, 1, 500, pipe_reader);
    	// LOG_I("Here IS the process:\n %s", cmdResult);

    	// Get delimited strings.
    	parsed = strtok(cmdResult, delimiters);

    	for (int i = 0; parsed != NULL; i++)
    	{
    		if (i == 8) {
    			// LOG_I("-->This is PID: %s",parsed);
    			strncpy(pid, parsed, size);
    		}
    		// Needed to avoid deadloop
    		parsed = strtok (NULL, " ");
    	}

    	free (cmdResult);
    	pclose(pipe_reader);
    }

    /*
     * Checks if mcDriverDaemon is alive.
     * returns 0 if dead and 1 if alive
     */
    bool isDaemonAlive(void) 
    {

    	// Read only value
    	char * daemonStr = (char *)"#mcdaemon";
        struct sockaddr_un remote; /**< Remote address */
    	int32_t socketDescriptor = 0; /**< Local socket descriptor */
    	int32_t len;

    	// LOG_I(" Trying to %s socket", daemonStr);

    	remote.sun_family = AF_UNIX;

    	strncpy((char *)&(remote.sun_path), (char *)daemonStr, sizeof(remote.sun_path));

        if ((socketDescriptor = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            // LOG_E("Can't open stream socket.");
            return false;
        }

        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        // The Daemon socket is in the Abstract Domain(LINUX ONLY!)
        remote.sun_path[0] = 0;
        if (::connect(socketDescriptor, (struct sockaddr *) &remote, len) < 0) {
            // LOG_E("connect()");
            // LOG_I("Daemon is dead");
            return false;
        }
        // LOG_I("Daemon is alive");
        return true;
    }

} // namespace DaemonControl

} // namespace TestUtils


