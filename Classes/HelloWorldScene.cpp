#include "HelloWorldScene.h"
#include "texpoly.h"
#include "B2DebugDrawLayer.h"

#include "clipper.hpp"

#define PTM_RATIO 32

USING_NS_CC;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback));
    
	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
                                origin.y + pCloseItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    /// Load the background sprite
    if (CCSprite* pSprite = CCSprite::create("wood_pattern.png")) {
        // position the sprite on the center of the screen
        pSprite->setPosition(ccp(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
        // add the sprite as a child to this layer
        addChild(pSprite, 0);
    }

    // Load the pix2d logo
    if (CCSprite *logo = CCSprite::create("banner.png")) {
        logo->setAnchorPoint(ccp(0, 1));
        logo->setPosition(ccp(0, visibleSize.height));
        addChild(logo);
    }

    /// Init random seed
    srand(time(NULL));

    /// Create the physics world
    mWorld = new b2World(b2Vec2(0, -10));

    /// Initialize the physics
    initPhysics();

    /// Add the box2d debug draw layer
    /// addChild(B2DebugDrawLayer::create(mWorld, 32),INT_MAX);

    scheduleUpdate();

    /*
    /// Example of a concave polygon with hole
    int p[] = {80,200,112,120,160,72,256,88,336,120,368,248,352,296,272,312,256,248,192,216,160,232,112,280,64,248};
    CCPointVector path;
    for (int i = 0; i < 24; i += 2)
        path.push_back(ccp(p[i],p[i+1]));

    int h[] = {144,168,192,120,272,136,320,200,288,232,240,168,192,184};
    CCPointVector hole;
    for (int i = 0; i < 14; i += 2)
        hole.push_back(ccp(h[i],h[i+1]));

    /// Create the polygon and add it to the layer
    addChild(TexPoly::create(path, hole, "pattern.png", mWorld));
    */
    return true;
}

void HelloWorld::initPhysics()
{
    // Create the world edges
    b2BodyDef bd;
    bd.position.SetZero();
    bd.type = b2_staticBody;

    /// Edge shape
    b2EdgeShape edge;

    /// Fixture definition
    b2FixtureDef fd;
    fd.shape = &edge;

    /// Get the world instance
    b2Body *body = mWorld->CreateBody(&bd);

    CCSize winSize = CCDirector::sharedDirector()->getWinSizeInPixels();

    edge.Set(b2Vec2_zero, b2Vec2(winSize.width / PTM_RATIO, 0));
    body->CreateFixture(&fd);

    for (int i = 0; i < 10; ++i) {
        int size = 3 + (rand() % 7); /// Min 3. Max 10.
        /// Texture size must be power of two
        if (CCNode *node = createRandomPolygon(size, "pattern.png"))
            addChild(node);
    }
}

#define POLY_MAX_SIZE 200

CCNode *HelloWorld::createRandomPolygon(int size, std::string filename)
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSizeInPixels();

    /// Polygon's origin point
    ClipperLib::IntPoint pos(winSize.width * CCRANDOM_0_1(), winSize.height * CCRANDOM_0_1());

    ClipperLib::Path in;
    for (int i = 0; i < size; ++i)
        in.push_back(ClipperLib::IntPoint(pos.X + (POLY_MAX_SIZE * CCRANDOM_0_1()), pos.Y + (POLY_MAX_SIZE * CCRANDOM_0_1())));

    /// Simplify the polygon and turns complex polygon into simple polygon
    ClipperLib::Paths out;
    ClipperLib::SimplifyPolygon(in, out);

    if (out.size() < 1)
        return NULL; /// No polygon

    ClipperLib::Path path = out.at(0);          /// TODO: manage all polygons

    /// Convert ClipperLib::IntPoint into cocos2d::CCPoint
    CCPointVector points;
    for (const ClipperLib::IntPoint &p : path)
        points.push_back(ccp(p.X, p.Y));

    /// We can safely create the polygon
    TexPoly *poly = TexPoly::create(points, filename, mWorld);
    /// Set a random color. RGBA values must be from 0 to 1
    /// poly->setColor(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.f));

    return poly;
}

void HelloWorld::update(float dt)
{
    b2Body* next = mWorld->GetBodyList();

    while (next) {
        b2Body* body = next;
        if (body->GetType() != b2_staticBody) {
            /// Get the cocos2d node attached to the box2d physics body
            CCNode *node = static_cast<CCNode *>(body->GetUserData());
            /// Update both CCNode's position and rotation
            node->setPosition(ccp(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO));
            node->setRotation( -1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
        }
        next = body->GetNext();
    }

    /// Update the physics world
    mWorld->Step(1.f / 60.f, 8, 3);
    mWorld->ClearForces();
}

void HelloWorld::menuCloseCallback(CCObject* pSender)
{
    /// Clean up physics world (remove all bodies)
    b2Body* next = mWorld->GetBodyList();

    /// Destroy all physics bodies and remove attached CCNode objects
    while (next) {
        b2Body* body = next;
        if (body->GetUserData())
            removeChild(static_cast<CCNode *>(body->GetUserData()), true);
        mWorld->DestroyBody(body);
        next = body->GetNext();
    }

    initPhysics();
}
