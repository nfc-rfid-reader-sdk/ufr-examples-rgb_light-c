/*
 ============================================================================
 Project Name: project_name
 Name        : file_name.c
 Author      : d-logic (http://www.d-logic.net/nfc-rfid-reader-sdk/)
 Version     :
 Copyright   : 2017.
 Description : Project in C (Language standard: c99)
 Dependencies: uFR firmware - min. version x.y.z {define in ini.h}
               uFRCoder library - min. version x.y.z {define in ini.h}
 ============================================================================
 */

/* includes:
 * stdio.h & stdlib.h are included by default (for printf and LARGE_INTEGER.QuadPart (long long) use %lld or %016llx).
 * inttypes.h, stdbool.h & string.h included for various type support and utilities.
 * conio.h is included for windows(dos) console input functions.
 * windows.h is needed for various timer functions (GetTickCount(), QueryPerformanceFrequency(), QueryPerformanceCounter())
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#if __WIN32 || __WIN64
#	include <conio.h>
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	define __USE_MISC
#	include <unistd.h>
#	include <termios.h>
#	undef __USE_MISC
#	include "conio_gnu.h"
#else
#	error "Unknown build platform."
#endif
#include <uFCoder.h>
#include "ini.h"
#include "uFR.h"
#include "utils.h"
//------------------------------------------------------------------------------
void usage(void);
void menu(char key);
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size);
void enable_color(void);
void disable_color(void);
//------------------------------------------------------------------------------
int main(void)
{
	char key;
	bool card_in_field = false;
	uint8_t old_sak = 0, old_uid_size = 0, old_uid[10];
	uint8_t sak, uid_size, uid[10];
	UFR_STATUS status;

	usage();
	printf(" --------------------------------------------------\n");
	printf("     Please wait while opening uFR NFC reader.\n");
	printf(" --------------------------------------------------\n");

#ifdef __DEBUG
	status = ReaderOpenEx(1, PORT_NAME, 1, NULL);
#else
	status = ReaderOpen();
#endif
	if (status != UFR_OK)
	{
		printf("Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
	status = ReaderReset();
	if (status != UFR_OK)
	{
		ReaderClose();
		printf("Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
#if __WIN32 || __WIN64
	Sleep(500);
#else // if linux || __linux__ || __APPLE__
	usleep(500000);
#endif

	if (!CheckDependencies())
	{
		ReaderClose();
		getchar();
		return EXIT_FAILURE;
	}

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

#if linux || __linux__ || __APPLE__
	_initTermios(0);
#endif
	do
	{
		while (!_kbhit())
		{
			status = GetCardIdEx(&sak, uid, &uid_size);
			switch (status)
			{
				case UFR_OK:
					if (card_in_field)
					{
						if (old_sak != sak || old_uid_size != uid_size || memcmp(old_uid, uid, uid_size))
						{
							old_sak = sak;
							old_uid_size = uid_size;
							memcpy(old_uid, uid, uid_size);
							NewCardInField(sak, uid, uid_size);
						}
					}
					else
					{
						old_sak = sak;
						old_uid_size = uid_size;
						memcpy(old_uid, uid, uid_size);
						NewCardInField(sak, uid, uid_size);
						card_in_field = true;
					}
					break;
				case UFR_NO_CARD:
					card_in_field = false;
					status = UFR_OK;
					break;
				default:
					ReaderClose();
					printf(" Fatal error while trying to read card, status is: 0x%08X\n", status);
					getchar();
#if linux || __linux__ || __APPLE__
					_resetTermios();
					tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
					return EXIT_FAILURE;
			}
#if __WIN32 || __WIN64
			Sleep(300);
#else // if linux || __linux__ || __APPLE__
			usleep(300000);
#endif
		}

		key = _getch();
		menu(key);
	}
	while (key != '\x1b');

	ReaderClose();
#if linux || __linux__ || __APPLE__
	_resetTermios();
	tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
void menu(char key)
{

	switch (key)
	{
		case '1':
			enable_color();
			break;

		case '2':
			disable_color();
			break;

		case '\x1b':
			break;

		default:
			usage();
			break;
	}
}
//------------------------------------------------------------------------------
void usage(void)
{
		printf(" +------------------------------------------------+\n"
			   " |              RGB LIGH UFR CLASSIC              |\n"
			   " |                 version "APP_VERSION"                    |\n"
			   " +------------------------------------------------+\n"
			   "                              For exit, hit escape.\n");
		printf(" --------------------------------------------------\n");
		printf("  (1) - Enable RGB color\n"
			   "  (2) - Disable RGB color\n");
}
//------------------------------------------------------------------------------
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size)
{
	UFR_STATUS status;
	uint8_t dl_card_type;

	status = GetDlogicCardType(&dl_card_type);
	if (status != UFR_OK)
		return status;

	printf(" \a-------------------------------------------------------------------\n");
	printf(" Card type: %s, sak = 0x%02X, uid[%d] = ", GetDlTypeName(dl_card_type), sak, uid_size);
	print_hex_ln(uid, uid_size, ":");
	printf(" -------------------------------------------------------------------\n");

	return UFR_OK;
}
//------------------------------------------------------------------------------
void enable_color(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                           ENABLE COLOR                              \n");
	printf(" -------------------------------------------------------------------\n");

	int red_int, green_int, blue_int, intensity_int;
	uint8_t red, green, blue, intensity;
	UFR_STATUS status;

	printf("\nEnter RED color value (0 - 255)\n");
	scanf("%d%*c", &red_int);
	red = red_int;

	printf("\nEnter GREEN color value (0 - 255)\n");
	scanf("%d%*c", &green_int);
	green = green_int;

	printf("\nEnter BLUE color value (0 - 255)\n");
	scanf("%d%*c", &blue_int);
	blue = blue_int;

	printf("\nEnter INTENSITY in percent (0 - 100)\n");
	scanf("%d%*c", &intensity_int);
	intensity = intensity_int;

	status = UfrRgbLightControl(red, green, blue, intensity, 1);

	if(status)
	{
		printf("\nSet RGB color failed\n");
		printf("Error code = %02X\n", status);
	}
	else
	{
		printf("\nSet RGB color successful\n");
		printf("\n");
	}
}
//------------------------------------------------------------------------------
void disable_color(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                           DISABLE COLOR                            \n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;

	status = UfrRgbLightControl(0, 0, 0, 0, 0);

	if(status)
	{
		printf("\nDisable RGB color failed\n");
		printf("Error code = %02X\n", status);
	}
	else
	{
		printf("\nDisable RGB color successful\n");
		printf("\n");
	}
}
//------------------------------------------------------------------------------
