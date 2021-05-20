#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "WinDivert\windivert.h"

#define DIVERT_QUEUE_LEN_MAX 8192
#define DIVERT_QUEUE_TIME_MAX 2048
#define DIVERT_PACKET_SIZE_MAX 0xFFFF

#define BYTESWAP16(x) ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))
#define BYTESWAP32(x) ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000))

struct Host
{
	char Name[256];
	unsigned int Address;
	unsigned short int Port;
};

struct Protocol
{
	char Name[256];
	size_t Length;
	size_t Occ[UCHAR_MAX + 1];
};

const int ProtocolCount = 2;

Protocol Protocols[ProtocolCount + 1];

char MainWallet[256] = {0}, NoDevFeeWallet[256] = {0}, NoDevFeeWorker[256] = {0};

int NoDevFeeProtocol = 0, LogLevel = 2, ShowConsole = 1, OutputDelay = 10;

Host MainHosts[256], DevFeeHosts[256], NoDevFeeHosts[256];

int MainHostCount = 0, DevFeeHostCount = 0, NoDevFeeHostCount = 0;

FILE *Log = 0;

static void CreateOccTable(const unsigned char *needle, size_t needleLength, size_t *occ)
{

	for (size_t i = 0; i < UCHAR_MAX + 1; ++i)
		occ[i] = needleLength;

	const size_t needleLengthMinus1 = needleLength - 1;

	for (size_t i = 0; i < needleLengthMinus1; ++i)
		occ[needle[i]] = needleLengthMinus1 - i;
}

inline const unsigned char* SearchBMH(const unsigned char *haystack, size_t haystackLength, const unsigned char *needle, size_t needleLength, const size_t *occ)
{
	if (needleLength > haystackLength)
		return 0;

	const size_t needleLengthMinus1 = needleLength - 1;

	const unsigned char lastNeedleChar = needle[needleLengthMinus1];

	size_t haystackPosition = 0;

	while (haystackPosition <= haystackLength - needleLength)
	{
		const unsigned char occChar = haystack[haystackPosition + needleLengthMinus1];

		if ((lastNeedleChar == occChar) && (memcmp(needle, haystack + haystackPosition, needleLengthMinus1) == 0))
			return haystack + haystackPosition;

		haystackPosition += occ[occChar];
	}

	return 0;
}

inline void Print(FILE *stream1, FILE *stream2, const char *format, ...)
{
	va_list list;

	va_start(list, format);

	if (stream1)
		vfprintf(stream1, format, list);

	if (stream2)
		vfprintf(stream2, format, list);

	va_end(list);
}

inline void PrintTime(FILE *stream1, FILE *stream2)
{
	time_t t = time(0);
	tm *local = localtime(&t);

	Print(stream1, stream2, "%02d.%02d.%04d %02d:%02d:%02d", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);
}



static void PrintVersion(const char *version)
{
	Print(stdout, Log, "-----------------------------------------------------------------------------------------\n");
	Print(stdout, Log, "                                  %s\n", version);
	Print(stdout, Log, "-----------------------------------------------------------------------------------------\n");

	PrintTime(stdout, Log);
	Print(stdout, Log, "\n");
}

static int SetPorts(const Host *hosts, int hostCount, unsigned short int *ports, int portCount)
{
	for (int i = 0; i < hostCount; ++i)
	{
		bool match = false;

		for (int j = 0; j < portCount; ++j)
		{
			if (ports[j] == hosts[i].Port)
			{
				match = true;

				break;
			}
		}

		if (!match)
			ports[portCount++] = hosts[i].Port;
	}

	return portCount;
}

static void SetFilter(char *filter)
{
	unsigned short int ports[256];

	int portCount = 0;

	portCount = SetPorts(MainHosts, MainHostCount, ports, portCount);
	portCount = SetPorts(DevFeeHosts, DevFeeHostCount, ports, portCount);
	portCount = SetPorts(NoDevFeeHosts, NoDevFeeHostCount, ports, portCount);

	strcpy(filter, "ip && tcp");

	if (portCount > 0)
	{
		strcat(filter, " && (inbound ? (");

		for (int i = 0; i < portCount; ++i)
			sprintf(filter + strlen(filter), "tcp.SrcPort == %u%s", BYTESWAP16(ports[i]), (i == portCount - 1) ? ") : (" : " || ");

		for (int i = 0; i < portCount; ++i)
			sprintf(filter + strlen(filter), "tcp.DstPort == %u%s", BYTESWAP16(ports[i]), (i == portCount - 1) ? "))" : " || ");
	}

}

static void SetProtocol(int index, const char *name)
{
	strcpy(Protocols[index].Name, name);

	Protocols[index].Length = strlen(name);

	CreateOccTable((unsigned char*) Protocols[index].Name, Protocols[index].Length, Protocols[index].Occ);
}

void CloseLog()
{
	if (Log)
	{
		fflush(Log);

		fclose(Log);

		Log = 0;
	}
}

int __stdcall ConsoleHandler(unsigned long int type)
{
	CloseLog();

	return 1;
}

unsigned long int __stdcall FlushThread(void *parameter)
{
	while (true)
	{
		fflush(Log);

		Sleep(OutputDelay * 1000);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	atexit(CloseLog);
		
	if (!ShowConsole)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	if (LogLevel >= 2)
	{
		Log = fopen("log.txt", "a");

		CreateThread(0, 0, FlushThread, 0, 0, 0);
	}

	if (LogLevel >= 1)
		PrintVersion("PhoenixMiner DevFee interceptor 1.1");

	if (!SetConsoleCtrlHandler(ConsoleHandler, 1))
		if (LogLevel >= 1)
			Print(stdout, Log, "Error SetConsoleCtrlHandler %d\n", GetLastError());

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		Print(stdout, Log, "Error WSAStartup %d\n", WSAGetLastError());

		system("pause");

		return EXIT_FAILURE;
	}


	WSACleanup();

	Print(stdout, Log, "[1/4] NoDevFee successfuly initialized.\n[2/4] Please start PhoenixMiner now to detect your wallet.");

	char filter[50000];

	SetFilter(filter);

	SetProtocol(0, "eth_submitLogin");
	SetProtocol(1, "eth_Login");
	SetProtocol(ProtocolCount, "0x");

	HANDLE handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, -1000, 0);

	if (handle == INVALID_HANDLE_VALUE)
	{
		Print(stdout, Log, "Error WinDivertOpen %d\n", GetLastError());

		system("pause");

		return EXIT_FAILURE;
	}

	if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_LEN, DIVERT_QUEUE_LEN_MAX))
			Print(stdout, Log, "Error WinDivertSetParam WINDIVERT_PARAM_QUEUE_LEN %d\n", GetLastError());

	if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_TIME, DIVERT_QUEUE_TIME_MAX))
			Print(stdout, Log, "Error WinDivertSetParam WINDIVERT_PARAM_QUEUE_TIME %d\n", GetLastError());

	WINDIVERT_ADDRESS address;

	unsigned char packet[DIVERT_PACKET_SIZE_MAX];

	unsigned int packetLength;

	WINDIVERT_IPHDR *ipHeader;
	WINDIVERT_TCPHDR *tcpHeader;

	char *data;

	unsigned int dataLength;

	unsigned long int srcAddress = 0, dstAddress = 0;
	unsigned short int srcPort = 0, dstPort = 0;

	char devFeeWallet[256] = {0};

	int rotate = 0;

	int index = 0;

	Host NoDevFeeHost;

	memcpy(&NoDevFeeHost, (NoDevFeeHostCount > 0) ? &NoDevFeeHosts[0] : &MainHosts[0], sizeof(NoDevFeeHost));

	while (true)
	{
		if (!WinDivertRecv(handle, packet, sizeof(packet), &address, &packetLength))
		{
				PrintTime(stdout, Log);
				Print(stdout, Log, " Error WinDivertRecv %d\n", GetLastError());
		}

		WinDivertHelperParsePacket(packet, packetLength, &ipHeader, 0, 0, 0, &tcpHeader, 0, (void**) &data, &dataLength);

		bool modified = false;


		if ((ipHeader) && (tcpHeader))
		{
			if (address.Direction == WINDIVERT_DIRECTION_OUTBOUND)
			{
				bool main = false;


				if ((data) && (dataLength > 0))
				{
					int protocol = -1;

					for (int i = 0; i < ProtocolCount; ++i)
					{
						
						if (SearchBMH((unsigned char*) data, dataLength, (unsigned char*) Protocols[i].Name, Protocols[i].Length, Protocols[i].Occ) != 0)
						{
							protocol = i;

							break;
						}
					}

					if (protocol != -1)
					{
						char *wallet = (char*) SearchBMH((unsigned char*) data, dataLength, (unsigned char*) Protocols[ProtocolCount].Name, Protocols[ProtocolCount].Length, Protocols[ProtocolCount].Occ);
						if (wallet != 0)
						{
							if ((strlen(MainWallet) < 42) || (strlen(NoDevFeeWallet) < 42))
							{
								memcpy(MainWallet, wallet, 42);
								memcpy(NoDevFeeWallet, wallet, 42);

								Print(stdout, Log, "\n[3/4] PhoenixMiner initialized. Detecting your wallet...");
								Print(stdout, Log, "\n[4/4] Setup done!\nYour NoDevFee Wallet: %s\n", NoDevFeeWallet); 
								Print(stdout, Log, "\nWaiting for the next DevFee mining...\n\n");
							}

							if (memcmp(wallet, MainWallet, 42) != 0)
							{
								memcpy(devFeeWallet, wallet, 42);
								memcpy(wallet, NoDevFeeWallet, 42); //where magic happens
								
								PrintTime(stdout, Log);
								Print(stdout, Log, "\nWallet intercepted!\nDevFeeWallet: %s\nNoDevFeeWallet: %s\n\n", devFeeWallet, NoDevFeeWallet);
								Print(stdout, Log, "Waiting for the next DevFee mining period...\n\n");
								modified = true;
							}
						}
					}
				}
			}
			else if (address.Direction == WINDIVERT_DIRECTION_INBOUND)
			{
				if ((ipHeader->SrcAddr == NoDevFeeHost.Address) && (ipHeader->DstAddr == srcAddress) && (tcpHeader->SrcPort == NoDevFeeHost.Port) && (tcpHeader->DstPort == srcPort))
				{
					ipHeader->SrcAddr = dstAddress;
					tcpHeader->SrcPort = dstPort;

					modified = true;
				}
			}
		}

		WinDivertHelperCalcChecksums(packet, packetLength, modified ? 0 : WINDIVERT_HELPER_NO_REPLACE);

		if (!WinDivertSend(handle, packet, packetLength, &address, 0))
		{
			if (LogLevel >= 1)
			{
				PrintTime(stdout, Log);
				Print(stdout, Log, " Error WinDivertSend %d\n", GetLastError());
			}
		}
	}

	return EXIT_SUCCESS;
}
