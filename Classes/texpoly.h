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

#ifndef TEXPOLY_H
#define TEXPOLY_H

#include <cocos2d.h>
#include "Box2D.h"

USING_NS_CC;

typedef std::vector<CCPoint> CCPointVector;

class TexPoly : public CCNode {
public:
    TexPoly();
    virtual ~TexPoly();
    /** 
     * @brief Create a physic texture polygon.
     * @param points    The polygon vertices. Must be a simple polygon (http://en.wikipedia.org/wiki/Simple_polygon) and points must not repeat.
     * @param texture   The texture filename. Must be a Power Of Two (POT) texture
     * @param world     Pointer to the box2d physic world
     */
    static TexPoly *create(const CCPointVector &points, const std::string &filename, b2World *world);
    static TexPoly *create(const CCPointVector &points, const CCPointVector &hole , const std::string &filename, b2World *world);
    virtual bool init(const CCPointVector &points, const CCPointVector &hole, const std::string &filename, b2World *world);
    virtual void draw();

    void setColor(const ccColor4F &color);

protected:
    template <class T> void freeContainer(T &cntr);

protected:
    std::vector<ccVertex3F> mVertexPos;
    std::vector<ccTex2F> mTexCoords;
    GLint mColorLocation;
    ccColor4F mColor;
    CCTexture2D *mTexture;
};

#endif // TEXPOLY_H
