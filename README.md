Texture polygon (texpoly)
=========================

This is a cocos2d-x project 

## Description

debugconsole is a simple server-client implementation which allows remote access to a cocos2dx application for development/debugging purpose. The goal is to be able to remotely run lua scripts at runtime and to avoid  compiling/packaging/launching the application when it's not necessary (e.g. updating spritesheet, sounds...).

The server part is a cocos2dx CCNode listenning on a network socket for lua script requests. The client part is a simple python script which will construct the lua script and send it to the console.

## Requirements

* cocos2d-x 2.2.1

## Installation and usage
* Clone the project in $COCOS2D_ROOT/projects
* Open either the proj.ios/texpoly.xcodeproj or proj.mac/texpoly.xcodeproj projects

## License

MIT License

## Credits
cocos2d-x
box2d
poly2tri
clipper
