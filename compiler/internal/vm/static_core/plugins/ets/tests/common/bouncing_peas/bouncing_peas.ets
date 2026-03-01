/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function _skoala_createRedrawerPeer(redrawer: Object/*any*/): long/*KNativePointer*/ { return 1; }
function _skoala_drawPicture(picture: long/*KNativePointer*/, data: long/*KNativePointer*/, cb: Object/*any*/, sync: boolean): void { }
function _skoala_getFrame(peer: long/*KNativePointer*/, a: int, b: int): long/*KNativePointer*/ { return 1; }
function _skoala_getFrameWidth(peer: long/*KNativePointer*/, frame: long/*KNativePointer*/): int { return 1; }
function _skoala_getFrameHeight(peer: long/*KNativePointer*/, frame: long/*KNativePointer*/): int { return 1; }
function _skoala_initRedrawer(width: int, height: int, scale: float, peer: long/*KNativePointer*/, frame: long/*KNativePointer*/): long/*KNativePointer*/ { return 1; }
function _skoala_providePeerFactory(func: long/*KNativePointer*/, arg: long/*KNativePointer*/): void { }
function _skoala_setPlatformAPI(api: Object/*any*/): void { }

function _skoala_Canvas__1nDrawDrawable(ptr: long/*KNativePointer*/, drawablePtr: long/*KNativePointer*/, matrixArr: long/*KFloatPtr*/): void { }
function _skoala_Canvas__1nRestore(ptr: long/*KNativePointer*/): void { }
function _skoala_Canvas__1nSave(ptr: long/*KNativePointer*/): int { return 1; }
function _skoala_Paint__1nMake(): long/*KNativePointer*/ { return 1; }
function _skoala_Paint__1nSetColor(ptr: long/*KNativePointer*/, color: int): void { }
function _skoala_PictureRecorder__1nBeginRecording(ptr: long/*KNativePointer*/, left: float, top: float, right: float, bottom: float): long/*KNativePointer*/ { return 1; }
function _skoala_PictureRecorder__1nFinishRecordingAsDrawable(ptr: long/*KNativePointer*/): long/*KNativePointer*/ { return 1; }
function _skoala_PictureRecorder__1nFinishRecordingAsPictureWithCull(ptr: long/*KNativePointer*/, left: float, top: float, right: float, bottom: float): long/*KNativePointer*/ { return 1; }
function _skoala_PictureRecorder__1nMake(): long/*KNativePointer*/ { return 1; }

function _skoala_Canvas__1nDrawOval(canvasPtr: long/*KNativePointer*/, left: float, top: float, right: float, bottom: float, paintPtr: long/*KNativePointer*/): void { }
function _skoala_paragraph_ParagraphBuilder__1nMake(paragraphStylePtr: long/*KNativePointer*/, fontCollectionPtr: long/*KNativePointer*/): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_ParagraphBuilder__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_ParagraphBuilder__1nPushStyle(ptr: long/*KNativePointer*/, textStylePtr: long/*KNativePointer*/): void { }
function _skoala_paragraph_ParagraphBuilder__1nAddText(ptr: long/*KNativePointer*/, textString: long/*KStringPtr*/): void { }
function _skoala_paragraph_ParagraphBuilder__1nBuild(ptr: long/*KNativePointer*/): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_FontCollection__1nMake(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_FontCollection__1nSetDefaultFontManager(ptr: long/*KNativePointer*/, fontManagerPtr: long/*KNativePointer*/, defaultFamilyNameStr: long/*KStringPtr*/): void { }
function _skoala_FontMgr__1nDefault(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_ParagraphStyle__1nMake(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_Paragraph__1nLayout(ptr: long/*KNativePointer*/, width: long): void { }
function _skoala_paragraph_Paragraph__1nPaint(ptr: long/*KNativePointer*/, canvasPtr: long/*KNativePointer*/, x: float, y: float): void { }
function _skoala_paragraph_TextStyle__1nMake(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_TextStyle__1nSetColor(ptr: long/*KNativePointer*/, color: int): void { }
function _skoala_paragraph_TextStyle__1nSetFontSize(ptr: long/*KNativePointer*/, size: float): void { }
function _skoala_ManagedString__1nMake(textStr: FixedArray<byte>/*KStringPtr*/): long/*KNativePointer*/ { return 1; }
function _skoala_Paint__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_impl_Managed__invokeFinalizer(finalizer: long/*KNativePointer*/, obj: long/*KNativePointer*/): void { }
function _skoala_impl_RefCnt__getFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_ParagraphStyle__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_TextStyle__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_paragraph_Paragraph__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }
function _skoala_ManagedString__1nGetFinalizer(): long/*KNativePointer*/ { return 1; }

function _getPeerFactory() : long/*KNativePointer*/ { return 1; }
function _getEngine() : long/*KNativePointer*/ { return 1; }

function _skoala_enqueue_run(redrawerPeerPtr: long/*KNativePointer*/) : void { }

const BALL_RADIUS: float = 16;
const BALL_SPEED: float = 2;
const BALLS_QUANTITY: int = 256;

let result_hash: float = 0;
function addToHash(val: float): void {
    if (val > 0) {
        result_hash += val;
    } else {
        result_hash -= val;
    }
}

function encodeFPS(input: int): FixedArray<byte>/*Uint8Array*/ {
    const HEADER_LEN: int = 4;
    let digitsQuantity = 0;
    for (let x = input; x > 0; x = x / 10) digitsQuantity++;
    let resultLength = digitsQuantity + HEADER_LEN + 1;
    let result : FixedArray<byte> = new FixedArray<byte>(resultLength);/*Uint8Array(resultLength);*/
    let index: int = HEADER_LEN;
    for (let digitNumber: int = 0; digitNumber < digitsQuantity; digitNumber++) {
        let cp = input / power(10., (digitsQuantity - digitNumber - 1).toDouble()).toInt() % 10 + 48;
        result[index++] = cp.toByte();
    }
    result[index] = 0;
    result[0] = ((resultLength - HEADER_LEN) & 0xff).toByte();
    result[0 + 1] = (((resultLength - HEADER_LEN) as int >> 8) & 0xff).toByte();
    result[0 + 2] = (((resultLength - HEADER_LEN) as int >> 16) & 0xff).toByte();
    result[0 + 3] = (((resultLength - HEADER_LEN) as int >> 24) & 0xff).toByte();
    return result;
}

class Ball {
    constructor() {}
    constructor(x: float, y: float, speed: float, radius: float) {
        this.radius = radius;

        this.x = x;
        this.y = y;
        this.vx = sqrt(2).toFloat() * speed;
        this.vy = sqrt(2).toFloat() * speed;
    }
    draw(canvasPtr: long/*KNativePointer*/): void {
        let shapePaintPtr = _skoala_Paint__1nMake();
        let paintFinalizer = _skoala_Paint__1nGetFinalizer();
        _skoala_Paint__1nSetColor(shapePaintPtr, 0xFF00FF00.toInt()/*KInt*/); // green
        _skoala_Canvas__1nDrawOval(canvasPtr, this.left(), this.top(), this.right(), this.bottom(), shapePaintPtr);
        _skoala_impl_Managed__invokeFinalizer(paintFinalizer, shapePaintPtr);
    }
    move(): void {
        this.x += this.vx;
        this.y += this.vy;
    }
    left(): float { return this.x - this.radius; }
    top(): float { return this.y - this.radius; }
    right(): float { return this.x + this.radius; }
    bottom(): float { return this.y + this.radius; }


    public x: float;
    public y: float;

    public vx: float;
    public vy: float;

    public radius: float;
}

class Drawer {
    constructor() {
        this.redrawerPeer = _skoala_createRedrawerPeer(new Object());
        this.frameHandle = _skoala_getFrame(this.redrawerPeer, 800, 600);

        this.frameWidth = _skoala_getFrameWidth(this.redrawerPeer, this.frameHandle);
        this.frameHeight = _skoala_getFrameHeight(this.redrawerPeer, this.frameHandle);

        this.pictureRecorderPtr = _skoala_PictureRecorder__1nMake();
        this.pictureRecorder2Ptr = _skoala_PictureRecorder__1nMake();

        const scale : float = 1;
        this.redrawerDataPtr = _skoala_initRedrawer(this.frameWidth, this.frameHeight, scale, this.redrawerPeer, this.frameHandle);

        this.startTime = Date.now();
    }

    draw(balls: Ball[]): void {
        let canvasPtr = _skoala_PictureRecorder__1nBeginRecording(this.pictureRecorderPtr, 0, 0, this.frameWidth.toFloat(), this.frameHeight.toFloat());

        _skoala_Canvas__1nSave(canvasPtr);
        let canvas2Ptr = _skoala_PictureRecorder__1nBeginRecording(this.pictureRecorder2Ptr, 0, 0, this.frameWidth.toFloat(), this.frameHeight.toFloat());

        for (let i = 0; i < balls.length; i++) {
            balls[i].draw(canvas2Ptr);
            addToHash(balls[i].x);
            addToHash(balls[i].y);
        }

        this.drawFPS(canvas2Ptr);

        let resourcePtr = _skoala_PictureRecorder__1nFinishRecordingAsDrawable(this.pictureRecorder2Ptr);
        let resourceFinalizer = _skoala_impl_RefCnt__getFinalizer();
        _skoala_Canvas__1nDrawDrawable(canvasPtr, resourcePtr, 0);

        _skoala_Canvas__1nRestore(canvasPtr);

        let picturePtr = _skoala_PictureRecorder__1nFinishRecordingAsPictureWithCull(this.pictureRecorderPtr, 0, 0, this.frameWidth.toFloat(), this.frameHeight.toFloat());
        let pictureFinalizer = _skoala_impl_RefCnt__getFinalizer();

        _skoala_drawPicture(picturePtr, this.redrawerDataPtr, new Object(), true);

        _skoala_impl_Managed__invokeFinalizer(resourceFinalizer, resourcePtr);
        _skoala_impl_Managed__invokeFinalizer(pictureFinalizer, picturePtr);

        // Update FPS
        this.drawCounter++;
        const currentTime: long = Date.now();
        if (currentTime - this.startTime >= 1000) {
            this.fps = ((this.drawCounter * 1000) as long / (currentTime - this.startTime) as long).toInt();
            this.drawCounter = 0;
            this.startTime = Date.now();
        }
    }

    private drawFPS(canvasPtr: long/*KNativePointer*/): void {
        const fontCollectionPtr = _skoala_paragraph_FontCollection__1nMake();
        const fontCollectionFinalizer = _skoala_impl_RefCnt__getFinalizer();

        const fontMgrPtr = _skoala_FontMgr__1nDefault();
        const fontMgrFinalizer = _skoala_impl_RefCnt__getFinalizer();

        _skoala_paragraph_FontCollection__1nSetDefaultFontManager(fontCollectionPtr, fontMgrPtr, 0);
        const paragraphStylePtr = _skoala_paragraph_ParagraphStyle__1nMake();
        const paragraphStyleFinalizer = _skoala_paragraph_ParagraphStyle__1nGetFinalizer();

        const paragraphBuilderPtr = _skoala_paragraph_ParagraphBuilder__1nMake(paragraphStylePtr, fontCollectionPtr);
        const paragraphBuilderFinalizer = _skoala_paragraph_ParagraphBuilder__1nGetFinalizer();

        const textStylePtr = _skoala_paragraph_TextStyle__1nMake();
        const textStyleFinalizer = _skoala_paragraph_TextStyle__1nGetFinalizer()

        _skoala_paragraph_TextStyle__1nSetColor(textStylePtr, 0xffff0000.toInt()/*KInt*/);
        _skoala_paragraph_TextStyle__1nSetFontSize(textStylePtr, 64);
        _skoala_paragraph_ParagraphBuilder__1nPushStyle(paragraphBuilderPtr, textStylePtr);

        const encodedText = encodeFPS(this.fps);

        const textPtr = _skoala_ManagedString__1nMake(encodedText);
        const textFinalizer = _skoala_ManagedString__1nGetFinalizer();

        _skoala_paragraph_ParagraphBuilder__1nAddText(paragraphBuilderPtr, textPtr);

        const paragraphPtr = _skoala_paragraph_ParagraphBuilder__1nBuild(paragraphBuilderPtr);
        const paragraphFinalizer = _skoala_paragraph_Paragraph__1nGetFinalizer()
        _skoala_paragraph_Paragraph__1nLayout(paragraphPtr, /*Infinity*/500/*KFloat*/);

        _skoala_paragraph_Paragraph__1nPaint(paragraphPtr, canvasPtr, 100, 100);

        _skoala_impl_Managed__invokeFinalizer(paragraphFinalizer, paragraphPtr);
        _skoala_impl_Managed__invokeFinalizer(paragraphBuilderFinalizer, paragraphBuilderPtr);
        _skoala_impl_Managed__invokeFinalizer(textFinalizer, textPtr);
        _skoala_impl_Managed__invokeFinalizer(textStyleFinalizer, textStylePtr);
        _skoala_impl_Managed__invokeFinalizer(paragraphStyleFinalizer, paragraphStylePtr);
        _skoala_impl_Managed__invokeFinalizer(fontCollectionFinalizer, fontCollectionPtr);
        _skoala_impl_Managed__invokeFinalizer(fontMgrFinalizer, fontMgrPtr);
    }

    public frameWidth: int;
    public frameHeight: int;

    public redrawerPeer: long/*KNativePointer*/;

    private frameHandle: long/*KNativePointer*/;
    private pictureRecorderPtr: long/*KNativePointer*/;
    private pictureRecorder2Ptr: long/*KNativePointer*/;
    private redrawerDataPtr: long/*KNativePointer*/;

    private drawCounter: long = 0;
    private startTime: long = 0;
    private fps: int = 0;
}

class Scene {
    constructor(ballsQuantity: int, ballRadius: float) {
        this.drawer = new Drawer();

        this.balls = new Ball[ballsQuantity];
        let offset: float = 0;
        for (let i = 0; i < ballsQuantity; i++) {
            offset += ballRadius * 3;
            let x: float = offset % this.drawer.frameWidth;
            let y: float = ((offset / this.drawer.frameWidth).toInt() + 1).toFloat() * ballRadius * 3;
            this.balls[i] = new Ball(x, y, BALL_SPEED, ballRadius);
        }
    }

    nextFrame(): void {
        for (let i = 0; i < this.balls.length; i++) {
            this.balls[i].move();
        }

        // Compute collisions between balls
        for (let i = 0; i < this.balls.length; ++i) {
            for (let j = 0; j < this.balls.length; ++j) {
                if (i == j) continue;
                let b1 = this.balls[i];
                let b2 = this.balls[j];

                let nx: float = b2.x - b1.x;
                let ny: float = b2.y - b1.y;

                let dist: float = sqrt(nx * nx + ny * ny).toFloat();
                if (dist > (b1.radius + b2.radius)) continue;

                nx = nx / dist;
                ny = ny / dist;

                let dot: float = b1.vx * nx + b1.vy * ny;
                b1.vx -= nx * 2 * dot;
                b1.vy -= ny * 2 * dot;

                let d: float = b1.radius + b2.radius - dist - 0.1.toFloat();
                b1.x -= nx * d;
                b1.y -= ny * d;
            }
        }
        // Compute collisions with walls
        for (let i = 0; i < this.balls.length; i++) {
            let b = this.balls[i];
            if (b.left() <= 0 || b.right() >= this.drawer.frameWidth) {
                b.vx = -b.vx;
                b.x = b.left() <= 0 ? b.radius : this.drawer.frameWidth - b.radius;
            }
            if (b.top() <= 0 || b.bottom() >= this.drawer.frameHeight) {
                b.vy = -b.vy;
                b.y = b.top() <= 0 ? b.radius : this.drawer.frameHeight - b.radius;
            }
        }

        this.drawer.draw(this.balls);
    }

    run(): void {
        this.nextFrame();
        _skoala_enqueue_run(this.drawer.redrawerPeer);
    }

    private drawer: Drawer;
    private balls: Ball[];
}

let gScene: Scene;
class PlatformAPI {
    public run(): void {
        gScene.run();
    }
};
function platformAPI(): PlatformAPI {
    return new PlatformAPI();
}

function startApplication(): void {
    _skoala_providePeerFactory(_getPeerFactory(), _getEngine());
    _skoala_setPlatformAPI(platformAPI());

    gScene = new Scene(BALLS_QUANTITY, BALL_RADIUS);
    gScene.run();
}

function main(): int {
    startApplication();
    if (result_hash.toInt() != 7936)
    {
        console.print("Wrong result hash");
        console.println();
        return 1;
    }
    return 0;
}
