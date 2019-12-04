#!/bin/bash

set -u

cat > src/Website.h <<EOF
#ifndef _WEBSITE_H_
#define _WEBSITE_H_

#include <ESP8266WebServer.h>

/*************************************************************************
 *                    *** WARNING - AUTO-GENERATED ***                   *
 *************************************************************************
 *
 * This file is generated from Website.html
 *
 * Two methods to generate this file:
 *   1. Perform the search and replace below on Website.html
 *   2. Run Website.sh
 *
 * Search and replace:
 *   find " and replace   \"
 *   find ^ and replace     restServer.sendContent_P(PSTR("
 *   find $ and replace   \n"));
 *
 * As regex:
 *   s/"/\\\\"/g
 *   s/^/  restServer.sendContent_P(PSTR("/
 *   s/$/\\\\n"));/
 */

void sendWebsite(ESP8266WebServer &restServer) {
  restServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  restServer.sendHeader("Pragma", "no-cache");
  restServer.sendHeader("Expires", "-1");
  restServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  restServer.send(200, "text/html", "");

  // Begin Website.html
EOF

sed -e '
  s/"/\\"/g;
  s/^/  restServer.sendContent_P(PSTR("/;
  s/$/\\n"));/;' < Website.html >> src/Website.h

cat >> src/Website.h <<EOF
  // End Website.html

  restServer.sendContent("");
  restServer.client().stop();
};
#endif
EOF
