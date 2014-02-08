/**
 * Copyright (C) 2012-2014 Laurent Zubiaur - pix2d.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "texpoly.h"
#include "poly2tri.h"

#define PTM_RATIO 32

TexPoly::TexPoly()
: mTexture(nullptr)
{
    mColor = ccc4f(1.f, 1.f, 1.f, 1.f);
}

TexPoly::~TexPoly()
{
    CC_SAFE_RELEASE(mTexture);
}

TexPoly *TexPoly::create(const CCPointVector &points, const std::string &filename, b2World *world)
{
    CCPointVector empty;
    return create(points, empty, filename, world);
}

TexPoly *TexPoly::create(const CCPointVector &points, const CCPointVector &hole, const std::string &filename, b2World *world)
{
    TexPoly *ptr = new TexPoly();
    if (ptr && ptr->init(points, hole, filename, world)) {
        ptr->autorelease();
        return ptr;
    }
    delete ptr;
    return nullptr;
}

bool TexPoly::init(const CCPointVector &points, const CCPointVector &hole, const std::string &filename, b2World *world)
{
    if (filename.length() == 0) {
        CCLOGERROR("ERROR: Invalid texture filename (empty string)");
        return false;
    }

    /* STEP 1: Load texture into cache (if not already loaded) */
    mTexture = CCTextureCache::sharedTextureCache()->addImage(filename.c_str());
    if (! mTexture) {
        CCLOGERROR("ERROR: Can't load the texture '%s'", filename.c_str());
        return false;
    }

    /**
     * STEP 2: convertion from CCPoint to p2t::Point
     */
    std::vector<p2t::Point*> polyline;
    for (const CCPoint& p : points)
        polyline.push_back(new p2t::Point(p.x, p.y));

    std::vector<p2t::Point*> holePolyline;
    for (const CCPoint& p : hole)
        holePolyline.push_back(new p2t::Point(p.x, p.y));

    /**
     * STEP 3: Create CDT and add primary polyline
     * NOTE: polyline must be a simple polygon. The polyline's points
     * constitute constrained edges. No repeat points!!!
     */
    p2t::CDT cdt(polyline);

    /// Add the hole
    if (holePolyline.size())
        cdt.AddHole(holePolyline);

    /**
     * STEP 4: Triangulate!
     */
    cdt.Triangulate();

    std::vector<p2t::Triangle*> triangles;
    triangles = cdt.GetTriangles();

    /**
     * STEP 5: create the Box2D body
     */
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = b2Vec2_zero;
    bd.userData = this;
    b2Body *body = world->CreateBody(&bd);

    /**
     * STEP 6: Create the fixtures
     */
    /// The box2d polygon shape
    b2PolygonShape shape;
    /// The fixture definition
    b2FixtureDef fd;
    fd.density = 1.f;
    fd.restitution = .2f;
    fd.shape = &shape;
    /// The polygon shape's vertices (actually a triangle)
    b2Vec2 v[3];
    /// Texture size in pixels
    CCSize texSize = mTexture->getContentSizeInPixels();

    /// Iterate over all triangles
    for (p2t::Triangle *tri : triangles) {
        /// Iterate over the 3 triangle's points
        for (int i = 0; i < 3; ++i) {
            const p2t::Point &p = *tri->GetPoint(i);
            /// Convert from pixels to meters
            v[i].Set(p.x / PTM_RATIO, p.y / PTM_RATIO);
            /// Populate the vertices position
            mVertexPos.push_back(vertex3(p.x, p.y, .0f));   /// x y z
            /// Populate the texture coordinates
            mTexCoords.push_back(tex2(p.x / texSize.width, 1 - p.y / texSize.height));
        }
        /// Create the fixture
        shape.Set(v, 3);
        body->CreateFixture(&fd);
    }

    /**
     * STEP 7: Configure the shader program
     */
    /// Enable texture repeat
    ccTexParams params = {GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT};
    mTexture->setTexParameters(&params);
    mTexture->retain();

    /// Create the shader program
    CCGLProgram *program = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture_uColor);
    mColorLocation = glGetUniformLocation(program->getProgram(), "u_color");
    setShaderProgram(program);

    /**
     * STEP 8: cleanup
     */
    freeContainer(polyline);
    freeContainer(holePolyline);

    return true; /// Success
}


void TexPoly::draw()
{
    if (! isVisible()) return;

    /// Setup the OpenGL shader
    CC_NODE_DRAW_SETUP();
    /// Bind the 2D texture
    ccGLBindTexture2D(mTexture->getName());
    /// Enable shader attributes
    ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position | kCCVertexAttribFlag_TexCoords);

    /// Color
    getShaderProgram()->setUniformLocationWith4f(mColorLocation, mColor.r, mColor.g, mColor.b, mColor.a);
    /// Vertex
    glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(ccVertex3F), (void*)&mVertexPos[0]);
    /// TexCoords
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ccTex2F), (void*)&mTexCoords[0]);
    /// Available mode: GL_TRIANGLES, GL_TRIANGLES_STRIP and GL_TRIANGLE_FAN
    glDrawArrays(GL_TRIANGLES, 0, mVertexPos.size());

    CHECK_GL_ERROR_DEBUG();
    
    CC_INCREMENT_GL_DRAWS(1);
}

void TexPoly::setColor(const ccColor4F &color)
{
    mColor = color;
}

template <class T> void TexPoly::freeContainer(T &cntr)
{
    for (typename T::iterator it = cntr.begin(); it != cntr.end(); ++it)
        delete * it;
    cntr.clear();
}

