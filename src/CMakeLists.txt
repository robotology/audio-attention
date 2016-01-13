
# Copyright: (C) 2016 IIT-RBCS Robotics Brain and Cognitive Science and Tata Lab Lethbridge University
# Authors: Francesco Rea, Matthew Tata
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
cmake_minimum_required(VERSION 2.6)

if(OpenCV_FOUND)


#add_subdirectory( iKartFollower            )

#add_subdirectory( opOpticalFlow            )
#add_subdirectory( repeater                 )

else(OpenCV_FOUND)
    message(STATUS "OpenCV not found/selected, skipping modules")
endif(OpenCV_FOUND)


if(IPOPT_FOUND)
    #add_subdirectory( kalmanTest               )
    #add_subdirectory( predictorTest            )
else(IPOPT_FOUND)
    message(STATUS "IPOPT not found/selected, skipping gazeArbiter")
endif(IPOPT_FOUND)


