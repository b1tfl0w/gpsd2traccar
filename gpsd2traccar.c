#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <gps.h> /* sudo apt install libgps-dev */
#include <math.h>
#include <time.h>

#define SERVER_NAME         "localhost" /* address of the gps server */
#define SERVER_PORT         "2947" /* port of the gps server */

int debug = 0;
struct gps_data_t           g_gpsdata;
int                         ret;
void error(const char *msg) { perror(msg); exit(0); }

int portno =        5055; /* traccar osmand port */
char *host =        "localhost"; /* traccar address */
char *message_fmt = "POST ?id=%s&lat=%f&lon=%f&timestamp=%ld&altitude=%f&speed=%f&bearing=%f HTTP/1.0\r\n\r\n";
// set here your traccar device id
char *id = "furgone";
// set here how many seconds to wait for check and send gps data
int beatRateSecs = 10;
timespec_t timestamp;
time_t timeOfLastBeat = 0;
time_t timeNow = 0;
struct hostent *server;
struct sockaddr_in serv_addr;
int sockfd, bytes, sent, received, total;
char message[1024],response[4096];


int sendreq( char* id, double latitude, double longitude, timespec_t timestamp, float altitude, float speed, float bearing )
{

	/* fill in the parameters */
	sprintf(message,message_fmt,id,latitude,longitude,timestamp,altitude,speed,bearing);
	if (debug == 1) printf("Request:\n%s\n",message);

	/* create the socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");

	/* lookup the ip address */
	server = gethostbyname(host);
	if (server == NULL) error("ERROR no such host");

	/* fill in the structure */
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

	/* connect the socket */
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	/* send the request */
	total = strlen(message);
	sent = 0;
	do {
		bytes = write(sockfd,message+sent,total-sent);
		if (bytes < 0)
			error("ERROR writing message to socket");
		if (bytes == 0)
			break;
		sent+=bytes;
	} while (sent < total);

	///* receive the response */
	//memset(response,0,sizeof(response));
	//total = sizeof(response)-1;
	//received = 0;
	//do {
	//    bytes = read(sockfd,response+received,total-received);
	//    if (bytes < 0)
	//        error("ERROR reading response from socket");
	//    if (bytes == 0)
	//        break;
	//    received+=bytes;
	//} while (received < total);

	///*
	// * if the number of received bytes is the total size of the
	// * array then we have run out of space to store the response
	// * and it hasn't all arrived yet - so that's a bad thing
	// */
	//if (received == total)
	//    error("ERROR storing complete response from socket");

	/* close the socket */
	close(sockfd);

	/* process response */
	//printf("Response:\n%s\n",response);

	return 0;
}

int main(int argc,char *argv[]) {

	// 1) Try GPS open
	ret = gps_open(SERVER_NAME,SERVER_PORT,&g_gpsdata);
	if(ret != 0) {
		printf("[GPS] Can't open...bye!\r\n");
		return -1;
	}

	// 2) Enable the JSON stream - we enable the watch as well
	(void)gps_stream(&g_gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);

	// 3) Wait for data from GPSD
	while (gps_waiting(&g_gpsdata, 5000000)) {
		//sleep(1);
		if (-1 == gps_read(&g_gpsdata, NULL, 0)) {
			printf("Read error!! Exiting...\r\n");
			break;
		}
		else {
			if(g_gpsdata.fix.mode >= MODE_2D) {
				if(isfinite(g_gpsdata.fix.latitude) && isfinite(g_gpsdata.fix.longitude)) {
					// printf("[GPS DATA] Latitude %lf, Longitude %lf, Altitude %lf, Used satellites %d, Mode %d\r\n" , g_gpsdata.fix.latitude, g_gpsdata.fix.longitude, g_gpsdata.fix.altitude, g_gpsdata.satellites_used, g_gpsdata.fix.mode);
					timeNow = time(NULL);
					if (timeNow - timeOfLastBeat >= beatRateSecs ){ 
						sendreq(id, g_gpsdata.fix.latitude, g_gpsdata.fix.longitude, g_gpsdata.fix.time, g_gpsdata.fix.altitude, g_gpsdata.fix.speed, g_gpsdata.fix.track );
						timeOfLastBeat = time(NULL);
					}
					if (debug == 1) printf ("timeOfLastBeat:%ld\n", timeOfLastBeat);
				}
				else {
					printf(".\n");
				}
			}
			else {
				printf("Waiting for fix...\r\n");
			}
		}
	}

	// Close gracefully...
	(void)gps_stream(&g_gpsdata, WATCH_DISABLE, NULL);
	(void)gps_close(&g_gpsdata);
	return 0;
}
