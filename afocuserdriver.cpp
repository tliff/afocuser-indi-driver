/*******************************************************************************
  Copyright(c) 2012 Jasem Mutlaq. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/
#include "afocuserdriver.h"
#include <indidevapi.h>

AFocuserDriver::AFocuserDriver(std::string device)
{
    debug=false;
    this->device = device;
}

AFocuserDriver::~AFocuserDriver()
{
}



bool AFocuserDriver::Connect()
{
  this->fd = open(device.c_str(), O_RDWR | O_NOCTTY);

  struct termios tty;
  struct termios tty_old;
  memset (&tty, 0, sizeof tty);

  /* Error Handling */
  if ( tcgetattr ( fd, &tty ) != 0 ) {
    std::cout << "cerr " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
  }

  /* Save old tty parameters */
  tty_old = tty;

  /* Set Baud Rate */
  cfsetospeed (&tty, (speed_t)B9600);
  cfsetispeed (&tty, (speed_t)B9600);

  /* Setting other Port Stuff */
  tty.c_cflag     &=  ~PARENB;            // Make 8n1
  tty.c_cflag     &=  ~CSTOPB;
  tty.c_cflag     &=  ~CSIZE;
  tty.c_cflag     |=  CS8;

  tty.c_cflag     &=  ~CRTSCTS;           // no flow control
  tty.c_cc[VMIN]   =  1;                  // read doesn't block
  tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
  tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
  tty.c_cflag &= ~HUPCL;   // disable hang-up-on-close to avoid reset
  /* Make raw */
  cfmakeraw(&tty);

  /* Flush Port, then applies attributes */
  tcflush( fd, TCIFLUSH );
  if ( tcsetattr ( fd, TCSANOW, &tty ) != 0) {
    std::cerr << "Error " << errno << " from tcsetattr" << std::endl;
  }

  return this->fd != -1;

}

bool AFocuserDriver::Disconnect()
{
    close(this->fd);
    return true;
}

bool AFocuserDriver::move(int steps)
{
    int rc=0;
    std::string command;
    command += std::to_string(steps);
    command += '\n';

   IDLog("move command value is %s\n", command.c_str());

   rc = write_blocking(command);

   IDLog("startPulse returns %d\n",rc);
   if(rc==1)
      return true;

  return false;
}



bool AFocuserDriver::write_blocking(std::string s){
  this->write(s);
  IDLog("Waiting for confirmation\n");
  IDLog("FD is %i\n", this->fd);
  std::string res = this->wait_read();
  return res == "OK\n";
}

void AFocuserDriver::write(std::string s){
  IDLog("writing %s\n", s.c_str());
  const char *cmd = s.c_str();
  int n_written = 0,
  spot = 0;

  do {
    IDLog("Writing %d\n", cmd[spot]);
    n_written = ::write( fd, &cmd[spot], 1 );
    spot += n_written;
  } while (spot != s.length() && n_written > 0);
}

std::string AFocuserDriver::wait_read(){
  int n = 0;
  int spot = 0;
  char buf = '\0';

  /* Whole response*/
  char response[1024];
  memset(response, '\0', sizeof response);

  do {
    IDLog("reading\n");
    n = read( fd, &buf, 1 );
    sprintf( &response[spot], "%c", buf );
    spot += n;
  } while( buf != '\n' && n > 0);
  return std::string(response);
}
