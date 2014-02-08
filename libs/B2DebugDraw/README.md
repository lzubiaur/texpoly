B2DebugDraw
===========

Box2d debug draw for cocos2d-x.

We created a CCLayer to wrap debug drawing. By putting this layer on top, debug
drawing won't be overlaid by other CCNode.

#### Usage 

Just create a new instance of B2DebugDrawLayer, then add it to your CCLayer just as a normal CCNode.

Make sure to give it highest z-order.

    addChild(B2DebugDrawLayer::create(mB2World, PTM_RATIO), 9999);

#### Compatibility

Tested with cocos2d-x 2.1.2 using XCode 4.4
