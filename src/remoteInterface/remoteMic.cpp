/*
 * Copyright: (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick, Francesco Nori
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */



#include <stdio.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/AudioGrabberInterfaces.h>
#include <yarp/os/Property.h>
#include <yarp/os/NetInt16.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/Network.h>
#include <yarp/os/Port.h>
#include <yarp/os/Time.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/BufferedPort.h>

#define MICIFDEVICE "/dev/micif_dev"

#define STEREO 2
#define NUM_MICS (1 * STEREO)
#define SAMP_BUF_SIZE 256

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace std;

const double rec_seconds = 0.1;
const int rate = 48000;
const int fixedNSample = 4096;

int main(int argc, char *argv[]) {
  // initialization
  bool useDeviceDriver = false;
  bool usePortAudio    = true; // set default value
  string moduleName, robotName, robotPortName;
  Sound s;
  IAudioGrabberSound *get;
  int32_t buffermicif [NUM_MICS * SAMP_BUF_SIZE] = {0};

  // Open the network
  Network yarp;
  //BufferedPort<Sound> p;
  
  ResourceFinder rf;
  rf.setVerbose(true);
  rf.setDefaultConfigFile("remoteInterface.ini");      //overridden by --from parameter
  rf.setDefaultContext("remoteInterface");     //overridden by --context parameter
  rf.configure(argc,argv);
    
  /*********************************************************/
  
  if(rf.check("help")) {
    printf("HELP \n");
    printf("====== \n");
    printf("--name           : changes the rootname of the module ports \n");
    printf("--robot          : changes the name of the robot where the module interfaces to  \n");
    printf("--visualFeedback : indicates whether the visual feedback is active \n");
    printf("--name           : rootname for all the connection of the module \n");
    printf("--usePortAudio   : audio input from portaudio \n");
    printf("--useDeviceDriver: audio input from device driver \n");
    printf(" \n");
    printf("press CTRL-C to stop... \n");
    return true;
  }
  
  
  /* get the module name which will form the stem of all module port names */
  moduleName             = rf.check("name", 
				    Value("/remoteInterface"), 
				    "module name (string)").asString();
  
  /* detects whether the preferrable input is from portaudio */
  if(rf.check("usePortAudio")){
    printf("acquiring from portaudio \n");
  } 
  /* detects whether the preferrable input is from deviceDriver */
  if(rf.check("useDeviceDriver")){
    printf("acquiring from deviceDriver \n");
    usePortAudio = false;
    useDeviceDriver = true;
  } 
  /*
   * get the robot name which will form the stem of the robot ports names
   * and append the specific part and device required
   */
  robotName             = rf.check("robot", 
				   Value("icub"), 
				   "Robot name (string)").asString();
  robotPortName         = "/" + robotName + "/head";
  printf("robotName: %s \n", robotName.c_str());
  
  /*********************************************************/
  
  Port p;
  p.open("/sender");
  
  if(usePortAudio) {
    // Get a portaudio read device.
    Property conf;
    conf.put("device","portaudio");
    conf.put("read", "");
    // conf.put("samples", rate * rec_seconds);
    conf.put("samples", fixedNSample);
    conf.put("rate", rate);
    PolyDriver poly(conf);
    
    
    // Make sure we can read sound
    poly.view(get);
    if (get==NULL) {
      printf("cannot open interface");
      return 1;
    }
    else{
      printf("correctly opened the interface rate: %d, number of samples: %f, number of channels %d \n",rate, rate*rec_seconds, 2);
      //Grab and send
      
      get->startRecording(); //this is optional, the first get->getsound() will do this anyway
    }
  
    if(useDeviceDriver) {
      printf("reading from the device drive \n");
      // reading direclty from the device drive.
      int micif_dev = -1;
      micif_dev = open("/dev/micif_dev", O_RDONLY);
      if(micif_dev<0) {
	fprintf(stderr, "Error opening %s:", MICIFDEVICE);
	return -1;
      }
      // read from device
      int ExpectedReading = sizeof(int) * SAMP_BUF_SIZE; 
      if(read(micif_dev, buffermicif, ExpectedReading) < 0)
	return -1;
      else {
	printf("success in reading from the device \n");
	return 1;
      }
    }
    
    //Grab and send
    Sound s;
    get->startRecording(); //this is optional, the first get->getsound() will do this anyway.
  }
  
  Stamp ts;
  while (true)
    {
      double t1=yarp::os::Time::now();
      ts.update();  
      //s = p.prepare();           
      
      get->getSound(s);        
      
      //v1 = s.get(i,j);
      //v = (NetInt16) v1;
      //v2 = s.get(i+1,j+1); 
      //dataAnalysis = (short*) dataSound;        
      
      p.setEnvelope(ts);
      p.write(s);

      double t2=yarp::os::Time::now();
      printf("acquired %f seconds \n", t2-t1);
    }
    get->stopRecording();  //stops recording.

    return 0;
}

