/**
 * Web Server Module
 * 
 * HTTP server with REST API and embedded web UI.
 * Serves the control page at / and handles API calls at /api/
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

/**
 * Start the HTTP web server.
 * Must be called after wifi_init_ap().
 */
void webserver_start(void);

/**
 * Stop the HTTP web server.
 */
void webserver_stop(void);

#endif // WEBSERVER_H
