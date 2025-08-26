#include "Camera.h"
#include "EI2C.h"
#include "Time.h"
#include "IfxScuEru.h"
#include "IfxDma_Dma.h"
#include "IfxDma_reg.h"
#include "IfxDma.h"
#include "XinDongLib/Interrupts.h"

// (DO NOT EXTERN!!!) the three image buffers to catch every single frame
// each only writable by either Camera.c OR cv.c (static just to possess their uniqueness)
static uint16 g_Image1[CAM_IMAGE_HEIGHT][CAM_IMAGE_WIDTH];
static uint16 g_Image2[CAM_IMAGE_HEIGHT][CAM_IMAGE_WIDTH];
static uint16 g_Image3[CAM_IMAGE_HEIGHT][CAM_IMAGE_WIDTH];

uint16 (*writing_img_ptr)[CAM_IMAGE_WIDTH] = g_Image1;      // the image that is currently being received
uint16 (*occupied_img_ptr)[CAM_IMAGE_WIDTH] = 0;                // the image that is currently being processed by CV
uint16 (*latest_img_ptr)[CAM_IMAGE_WIDTH] = 0;              // the last image received that hasn't been read yet

void* Camera_GetLatest(void) {
    // if the previous buffer is not released, do not give it another one
    if (occupied_img_ptr != 0)
        return 0;
    // otherwise, return the latest image buffer pointer
    occupied_img_ptr = latest_img_ptr;
    latest_img_ptr = 0;
    return occupied_img_ptr;
}

void* Camera_Release(uint16 (*img_ptr)[CAM_IMAGE_WIDTH]) {
    // if it attempts to release the correct pointer, then proceed
    if (img_ptr == occupied_img_ptr) {
        occupied_img_ptr = 0;
        return 0;
    }
    // otherwise, do not release
    return occupied_img_ptr;
}

void* _Camera_Image_Received(void) {
    uint16 (*temp_ptr)[CAM_IMAGE_WIDTH] = writing_img_ptr;
    // if there is a buffer occupied, then there is only one buffer available
    if (occupied_img_ptr != 0) {
        if (occupied_img_ptr == g_Image1) {
            if (writing_img_ptr == g_Image2)
                writing_img_ptr = g_Image3;
            else
                writing_img_ptr = g_Image2;
        } else if (occupied_img_ptr == g_Image2) {
            if (writing_img_ptr == g_Image1)
                writing_img_ptr = g_Image3;
            else
                writing_img_ptr = g_Image1;
        } else if (occupied_img_ptr == g_Image3) {
            if (writing_img_ptr == g_Image1)
                writing_img_ptr = g_Image2;
            else
                writing_img_ptr = g_Image1;
        } else {        // no image is occupied
            if (writing_img_ptr == g_Image1)
                writing_img_ptr = g_Image2;
            else if (writing_img_ptr == g_Image2)
                writing_img_ptr = g_Image3;
            else if (writing_img_ptr == g_Image3)
                writing_img_ptr = g_Image1;
        }
    }
    // assign the latest image pointer
    latest_img_ptr = temp_ptr;
    // and return the writing pointer
    return writing_img_ptr;
}





















/*********** 寄存器配置 **************/

/* ATK-MC2640 UXGA寄存器配置
 * 该模式下帧率为15FPS
 * UXGA: 1600*1200
 */
const uint8 atk_mc2640_init_uxga_cfg[][2] = {
    {0xFF, 0x00},
    {0x2C, 0xFF},
    {0x2E, 0xDF},
    {0xFF, 0x01},
    {0x3C, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0xA8},
    {0x13, 0xE5},
    {0x14, 0x48},
    {0x2C, 0x0C},
    {0x33, 0x78},
    {0x3A, 0x33},
    {0x3B, 0xFB},
    {0x3E, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
    {0x35, 0xDA},
    {0x22, 0x1A},
    {0x37, 0xC3},
    {0x23, 0x00},
    {0x34, 0xC0},
    {0x36, 0x1A},
    {0x06, 0x88},
    {0x07, 0xC0},
    {0x0D, 0x87},
    {0x0E, 0x41},
    {0x4C, 0x00},
    {0x48, 0x00},
    {0x5B, 0x00},
    {0x42, 0x03},
    {0x4A, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5C, 0x00},
    {0x63, 0x00},
    {0x46, 0x00},
    {0x0C, 0x3C},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7C, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6C, 0x00},
    {0x6D, 0x80},
    {0x6E, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xC1},
    {0x3D, 0x34},
    {0x5A, 0x57},
    {0x12, 0x00},
    {0x17, 0x11},
    {0x18, 0x75},
    {0x19, 0x01},
    {0x1A, 0x97},
    {0x32, 0x36},
    {0x03, 0x0F},
    {0x37, 0x40},
    {0x4F, 0xCA},
    {0x50, 0xA8},
    {0x5A, 0x23},
    {0x6D, 0x00},
    {0x6D, 0x38},
    {0xFF, 0x00},
    {0xE5, 0x7F},
    {0xF9, 0xC0},
    {0x41, 0x24},
    {0xE0, 0x14},
    {0x76, 0xFF},
    {0x33, 0xA0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4C, 0x00},
    {0x87, 0xD5},
    {0x88, 0x3F},
    {0xD7, 0x03},
    {0xD9, 0x10},
    {0xD3, 0x82},
    {0xC8, 0x08},
    {0xC9, 0x80},
    {0x7C, 0x00},
    {0x7D, 0x00},
    {0x7C, 0x03},
    {0x7D, 0x48},
    {0x7D, 0x48},
    {0x7C, 0x08},
    {0x7D, 0x20},
    {0x7D, 0x10},
    {0x7D, 0x0E},
    {0x90, 0x00},
    {0x91, 0x0E},
    {0x91, 0x1A},
    {0x91, 0x31},
    {0x91, 0x5A},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7E},
    {0x91, 0x88},
    {0x91, 0x8F},
    {0x91, 0x96},
    {0x91, 0xA3},
    {0x91, 0xAF},
    {0x91, 0xC4},
    {0x91, 0xD7},
    {0x91, 0xE8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xE3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0C},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xC3, 0xEF},
    {0xA4, 0x00},
    {0xA8, 0x00},
    {0xC5, 0x11},
    {0xC6, 0x51},
    {0xBF, 0x80},
    {0xC7, 0x10},
    {0xB6, 0x66},
    {0xB8, 0xA5},
    {0xB7, 0x64},
    {0xB9, 0x7C},
    {0xB3, 0xAF},
    {0xB4, 0x97},
    {0xB5, 0xFF},
    {0xB0, 0xC5},
    {0xB1, 0x94},
    {0xB2, 0x0F},
    {0xC4, 0x5C},
    {0xC0, 0xC8},
    {0xC1, 0x96},
    {0x8C, 0x00},
    {0x86, 0x3D},
    {0x50, 0x00},
    {0x51, 0x90},
    {0x52, 0x2C},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    {0x5A, 0x90},
    {0x5B, 0x2C},
    {0x5C, 0x05},
    {0xD3, 0x02},
    {0xC3, 0xED},
    {0x7F, 0x00},
    {0xDA, 0x09},
    {0xE5, 0x1F},
    {0xE1, 0x67},
    {0xE0, 0x00},
    {0xDD, 0x7F},
    {0x05, 0x00},
};

/* ATK-MC2640 SVGA寄存器配置
 * 该模式下帧率为30FPS
 * UXGA: 800*600
 */
const uint8 atk_mc2640_init_svga_cfg[][2] = {
    {0xFF, 0x00},
    {0x2C, 0xFF},
    {0x2E, 0xDF},
    {0xFF, 0x01},
    {0x3C, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0xA8},
    {0x13, 0xE5},
    {0x14, 0x48},
    {0x2C, 0x0C},
    {0x33, 0x78},
    {0x3A, 0x33},
    {0x3B, 0xFB},
    {0x3E, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
    {0x35, 0xDA},
    {0x22, 0x1A},
    {0x37, 0xC3},
    {0x23, 0x00},
    {0x34, 0xC0},
    {0x36, 0x1A},
    {0x06, 0x88},
    {0x07, 0xC0},
    {0x0D, 0x87},
    {0x0E, 0x41},
    {0x4C, 0x00},
    {0x48, 0x00},
    {0x5B, 0x00},
    {0x42, 0x03},
    {0x4A, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5C, 0x00},
    {0x63, 0x00},
    {0x46, 0x22},
    {0x0C, 0x3C},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7C, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6C, 0x00},
    {0x6D, 0x80},
    {0x6E, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xC1},
    {0x3D, 0x34},
    {0x5A, 0x57},
    {0x12, 0x40},
    {0x17, 0x11},
    {0x18, 0x43},
    {0x19, 0x00},
    {0x1A, 0x4B},
    {0x32, 0x09},
    {0x37, 0xC0},
    {0x4F, 0xCA},
    {0x50, 0xA8},
    {0x5A, 0x23},
    {0x6D, 0x00},
    {0x3D, 0x38},
    {0xFF, 0x00},
    {0xE5, 0x7F},
    {0xF9, 0xC0},
    {0x41, 0x24},
    {0xE0, 0x14},
    {0x76, 0xFF},
    {0x33, 0xA0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4C, 0x00},
    {0x87, 0xD5},
    {0x88, 0x3F},
    {0xD7, 0x03},
    {0xD9, 0x10},
    {0xD3, 0x82},
    {0xC8, 0x08},
    {0xC9, 0x80},
    {0x7C, 0x00},
    {0x7D, 0x00},
    {0x7C, 0x03},
    {0x7D, 0x48},
    {0x7D, 0x48},
    {0x7C, 0x08},
    {0x7D, 0x20},
    {0x7D, 0x10},
    {0x7D, 0x0E},
    {0x90, 0x00},
    {0x91, 0x0E},
    {0x91, 0x1A},
    {0x91, 0x31},
    {0x91, 0x5A},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7E},
    {0x91, 0x88},
    {0x91, 0x8F},
    {0x91, 0x96},
    {0x91, 0xA3},
    {0x91, 0xAF},
    {0x91, 0xC4},
    {0x91, 0xD7},
    {0x91, 0xE8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xE3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0C},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xC3, 0xED},
    {0xA4, 0x00},
    {0xA8, 0x00},
    {0xC5, 0x11},
    {0xC6, 0x51},
    {0xBF, 0x80},
    {0xC7, 0x10},
    {0xB6, 0x66},
    {0xB8, 0xA5},
    {0xB7, 0x64},
    {0xB9, 0x7C},
    {0xB3, 0xAF},
    {0xB4, 0x97},
    {0xB5, 0xFF},
    {0xB0, 0xC5},
    {0xB1, 0x94},
    {0xB2, 0x0F},
    {0xC4, 0x5C},
    {0xC0, 0x64},
    {0xC1, 0x4B},
    {0x8C, 0x00},
    {0x86, 0x3D},
    {0x50, 0x00},
    {0x51, 0xC8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x5A, 0xC8},
    {0x5B, 0x96},
    {0x5C, 0x00},
    {0xD3, 0x02},
    {0xC3, 0xED},
    {0x7F, 0x00},
    {0xDA, 0x09},
    {0xE5, 0x1F},
    {0xE1, 0x67},
    {0xE0, 0x00},
    {0xDD, 0x7F},
    {0x05, 0x00},
};

const uint8 atk_mc2640_set_yuv422_cfg[][2] = {
    {0xFF, 0x00},
    {0xDA, 0x10},
    {0xD7, 0x03},
    {0xDF, 0x00},
    {0x33, 0x80},
    {0x3C, 0x40},
    {0xE1, 0x77},
    {0x00, 0x00},
};

const uint8 atk_mc2640_set_jpeg_cfg[][2] = {
    {0xFF, 0x01},
    {0xE0, 0x14},
    {0xE1, 0x77},
    {0xE5, 0x1F},
    {0xD7, 0x03},
    {0xDA, 0x10},
    {0xE0, 0x00},
};

const uint8 atk_mc2640_set_rgb565_cfg[][2] = {
    {0xFF, 0x00},
    {0xDA, 0x09},
    {0xD7, 0x03},
    {0xDF, 0x02},
    {0x33, 0xA0},
    {0x3C, 0x00},
    {0xE1, 0x67},
    {0xFF, 0x01},
    {0xE0, 0x00},
    {0xE1, 0x00},
    {0xE5, 0x00},
    {0xD7, 0x00},
    {0xDA, 0x00},
    {0xE0, 0x00},
};





























/********* SCCB相关 ************/
/**
 * @brief       设置SCCB接口SDA引脚为输出模式
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_sccb_set_sda_output(void)
{
    IfxPort_setPinMode(CAM_SDA_PORT, CAM_SDA_PIN, IfxPort_Mode_outputPushPullGeneral);
}

/**
 * @brief       设置SCCB接口SDA引脚为输入模式
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_sccb_set_sda_input(void)
{
    IfxPort_setPinMode(CAM_SDA_PORT, CAM_SDA_PIN, IfxPort_Mode_inputPullUp);
}

/**
 * @brief       写SDA
 * @param       无
 * @retval      无
 */
static void _ATK_MC2640_SCCB_SDA(uint8 bit) {
    if (bit) IfxPort_setPinState(CAM_SDA_PORT, CAM_SDA_PIN, IfxPort_State_high);
    else IfxPort_setPinState(CAM_SDA_PORT, CAM_SDA_PIN, IfxPort_State_low);
}

/**
 * @brief       写SCL
 * @param       无
 * @retval      无
 */
static void _ATK_MC2640_SCCB_SCL(uint8 bit) {
    if (bit) IfxPort_setPinState(CAM_SCL_PORT, CAM_SCL_PIN, IfxPort_State_high);
    else IfxPort_setPinState(CAM_SCL_PORT, CAM_SCL_PIN, IfxPort_State_low);
}

/**
 * @brief       写RST
 * @param       无
 * @retval      无
 */
static void _ATK_MC2640_RST(uint8 bit) {
    if (bit) IfxPort_setPinState(CAM_RESET_PORT, CAM_RESET_PIN, IfxPort_State_high);
    else IfxPort_setPinState(CAM_RESET_PORT, CAM_RESET_PIN, IfxPort_State_low);
}

/**
 * @brief       写PWDN
 * @param       无
 * @retval      无
 */
static void _ATK_MC2640_PWDN(uint8 bit) {
    if (bit) IfxPort_setPinState(CAM_PWDN_PORT, CAM_PWDN_PIN, IfxPort_State_high);
    else IfxPort_setPinState(CAM_PWDN_PORT, CAM_PWDN_PIN, IfxPort_State_low);
}

/**
 * @brief       写FLASH
 * @param       无
 * @retval      无
 */
static void _ATK_MC2640_FLASH(uint8 bit) {
    if (bit) IfxPort_setPinState(CAM_FLASH_PORT, CAM_FLASH_PIN, IfxPort_State_high);
    else IfxPort_setPinState(CAM_FLASH_PORT, CAM_FLASH_PIN, IfxPort_State_low);
}

/**
 * @brief       读SDA
 * @param       无
 * @retval      无
 */
uint8 _ATK_MC2640_SCCB_READ_SDA() {
    uint8 bit = (uint8)IfxPort_getPinState(CAM_SDA_PORT, CAM_SDA_PIN);
    return bit;
}

/**
 * @brief       SCCB接口延时函数
 * @param       无
 * @retval      无
 */
static inline void _atk_mc2640_sccb_delay(void)
{
    Time_Delay_us(5);
}

/**
 * @brief       SCCB接口起始信号
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_sccb_start(void)
{
    _ATK_MC2640_SCCB_SDA(1);
    _ATK_MC2640_SCCB_SCL(1);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SDA(0);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(0);
}

/**
 * @brief       SCCB接口停止信号
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_sccb_stop(void)
{
    _ATK_MC2640_SCCB_SDA(0);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(1);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SDA(1);
    _atk_mc2640_sccb_delay();
}

/**
 * @brief       SCCB接口主机向从机写入一个字节数据
 * @param       dat: 待写入的一个字节数据
 * @retval      无
 */
static void _atk_mc2640_sccb_write_byte(uint8 dat)
{
    sint8 dat_index;
    uint8 dat_bit;

    for (dat_index=7; dat_index>=0; dat_index--)
    {
        dat_bit = (dat >> dat_index) & 0x01;
        _ATK_MC2640_SCCB_SDA(dat_bit);
        _atk_mc2640_sccb_delay();
        _ATK_MC2640_SCCB_SCL(1);
        _atk_mc2640_sccb_delay();
        _ATK_MC2640_SCCB_SCL(0);
    }

    _ATK_MC2640_SCCB_SDA(1);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(1);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(0);
}

/**
 * @brief       SCCB接口主机向从机读取一个字节数据
 * @param       dat: 读取到的一个字节数据
 * @retval      无
 */
static void _atk_mc2640_sccb_read_byte(uint8 *dat)
{
    sint8 dat_index;
    uint8 dat_bit;

    _atk_mc2640_sccb_set_sda_input();

    for (dat_index=7; dat_index>=0; dat_index--)
    {
        _atk_mc2640_sccb_delay();
        _ATK_MC2640_SCCB_SCL(1);
        dat_bit = _ATK_MC2640_SCCB_READ_SDA();
        *dat |= (dat_bit << dat_index);
        _atk_mc2640_sccb_delay();
        _ATK_MC2640_SCCB_SCL(0);
    }

    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(1);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SCL(0);
    _atk_mc2640_sccb_delay();
    _ATK_MC2640_SCCB_SDA(0);
    _atk_mc2640_sccb_delay();

    _atk_mc2640_sccb_set_sda_output();
}

/**
 * @brief       ATK-MC2640 SCCB接口初始化
 * @param       无
 * @retval      无
 */
void _atk_mc2640_sccb_init(void)
{
    IfxPort_setPinMode(CAM_SDA_PORT, CAM_SDA_PIN, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(CAM_SCL_PORT, CAM_SCL_PIN, IfxPort_Mode_outputPushPullGeneral);

    _atk_mc2640_sccb_stop();
}

/**
 * @brief       SCCB接口3阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 *              dat     : Write Data
 * @retval      无
 */
void _atk_mc2640_sccb_3_phase_write(uint8 id_addr, uint8 sub_addr, uint8 dat)
{
    _atk_mc2640_sccb_start();
    _atk_mc2640_sccb_write_byte((id_addr << 1) | ATK_MC2640_SCCB_WRITE);
    _atk_mc2640_sccb_write_byte(sub_addr);
    _atk_mc2640_sccb_write_byte(dat);
    _atk_mc2640_sccb_stop();
}

/**
 * @brief       SCCB接口2阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 * @retval      无
 */
void _atk_mc2640_sccb_2_phase_write(uint8 id_addr, uint8 sub_addr)
{
    _atk_mc2640_sccb_start();
    _atk_mc2640_sccb_write_byte((id_addr << 1) | ATK_MC2640_SCCB_WRITE);
    _atk_mc2640_sccb_write_byte(sub_addr);
    _atk_mc2640_sccb_stop();
}

/**
 * @brief       SCCB接口2阶段读传输
 * @param       id_addr: ID Address
 *              dat: 读取到的数据
 * @retval      无
 */
void _atk_mc2640_sccb_2_phase_read(uint8 id_addr, uint8 *dat)
{
    _atk_mc2640_sccb_start();
    _atk_mc2640_sccb_write_byte((id_addr << 1) | ATK_MC2640_SCCB_READ);
    _atk_mc2640_sccb_read_byte(dat);
    _atk_mc2640_sccb_stop();
}






















/********* Camera相关 **********/

/* ATK-MC2640寄存器块枚举 */
typedef enum
{
    ATK_MC2640_REG_BANK_DSP = 0x00, /* DSP寄存器块 */
    ATK_MC2640_REG_BANK_SENSOR,     /* Sensor寄存器块 */
} atk_mc2640_reg_bank_t;

/* ATK-MC2640模块数据结构体 */
static struct
{
    struct {
        uint16 width;
        uint16 height;
    } output;

} g_atk_mc2640_sta = {{0}};

/**
 * @brief       ATK-MC2640模块写寄存器
 * @param       reg: 寄存器地址
 *              dat: 待写入的值
 * @retval      无
 */
static void _atk_mc2640_write_reg(uint8 reg, uint8 dat)
{
    _atk_mc2640_sccb_3_phase_write(ATK_MC2640_SCCB_ADDR, reg, dat);
}

/**
 * @brief       ATK-MC2640模块读寄存器
 * @param       reg: 寄存器的地址
 * @retval      读取到的寄存器值
 */
static uint8 _atk_mc2640_read_reg(uint8 reg)
{
    uint8 dat = 0;

    _atk_mc2640_sccb_2_phase_write(ATK_MC2640_SCCB_ADDR, reg);
    _atk_mc2640_sccb_2_phase_read(ATK_MC2640_SCCB_ADDR, &dat);

    return dat;
}

/**
 * @brief       设置ATK-MC2640模块启用的寄存器块
 * @param       set: ATK_MC2640_REG_BANK_DSP   : DSP寄存器块
 *                   ATK_MC2640_REG_BANK_SENSOR: Sensor寄存器块
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块启用的寄存器块成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
static uint8 _atk_mc2640_reg_bank_select(atk_mc2640_reg_bank_t bank)
{
    switch (bank)
    {
        case ATK_MC2640_REG_BANK_DSP:
        {
            _atk_mc2640_write_reg(ATK_MC2640_REG_BANK_SEL, 0x00);
            break;
        }
        case ATK_MC2640_REG_BANK_SENSOR:
        {
            _atk_mc2640_write_reg(ATK_MC2640_REG_BANK_SEL, 0x01);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       ATK-MC2640模块硬件初始化
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_hw_init(void)
{
    IfxPort_setPinMode(CAM_VSYNC_HW_CIF_PIN.pin.port, CAM_VSYNC_HW_CIF_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_HSYNC_HW_CIF_PIN.pin.port, CAM_HSYNC_HW_CIF_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_PCLK_HW_CIF_PIN.pin.port, CAM_PCLK_HW_CIF_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_VSYNC_SW_EXTI_PIN.pin.port, CAM_VSYNC_SW_EXTI_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_HSYNC_SW_EXTI_PIN.pin.port, CAM_HSYNC_SW_EXTI_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_PCLK_SW_EXTI_PIN.pin.port, CAM_PCLK_SW_EXTI_PIN.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D0_CIF_PORT.pin.port, CAM_D0_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D1_CIF_PORT.pin.port, CAM_D1_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D2_CIF_PORT.pin.port, CAM_D2_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D3_CIF_PORT.pin.port, CAM_D3_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D4_CIF_PORT.pin.port, CAM_D4_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D5_CIF_PORT.pin.port, CAM_D5_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D6_CIF_PORT.pin.port, CAM_D6_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_D7_CIF_PORT.pin.port, CAM_D7_CIF_PORT.pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    IfxPort_setPinMode(CAM_RESET_PORT, CAM_RESET_PIN, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(CAM_PWDN_PORT, CAM_PWDN_PIN, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(CAM_FLASH_PORT, CAM_FLASH_PIN, IfxPort_Mode_outputPushPullGeneral);

    _ATK_MC2640_RST(1);
    _ATK_MC2640_PWDN(1);
    _ATK_MC2640_FLASH(0);
}

/**
 * @brief       ATK-MC2640模块退出掉电模式
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_exit_power_down(void)
{
    _ATK_MC2640_PWDN(0);
    Time_Delay_us(10000);
}

/**
 * @brief       ATK-MC2640模块硬件复位
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_hw_reset(void)
{
    _ATK_MC2640_RST(0);
    Time_Delay_us(10000);
    _ATK_MC2640_RST(1);
    Time_Delay_us(10000);
}

/**
 * @brief       ATK-MC2640模块软件复位
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_sw_reset(void)
{
    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM7, 0x80);
    Time_Delay_us(50000);
}

/**
 * @brief       获取ATK-MC2640模块制造商ID
 * @param       无
 * @retval      制造商ID
 */
static uint16 _atk_mc2640_get_mid(void)
{
    uint16 mid;

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    mid = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_MIDH) << 8;
    mid |= _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_MIDL);

    return mid;
}

/**
 * @brief       获取ATK-MC2640模块产品ID
 * @param       无
 * @retval      产品ID
 */
static uint16 _atk_mc2640_get_pid(void)
{
    uint16 pid;

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    pid = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_PIDH) << 8;
    pid |= _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_PIDL);

    return pid;
}

/**
 * @brief       初始化ATK-MC2640寄存器配置
 * @param       无
 * @retval      无
 */
static void _atk_mc2640_init_reg(void)
{
    uint32 cfg_index;
    uint8 zmow;
    uint8 zmoh;
    uint8 zmhh;

    for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_init_uxga_cfg)/sizeof(atk_mc2640_init_uxga_cfg[0])); cfg_index++)
    {
        _atk_mc2640_write_reg(atk_mc2640_init_uxga_cfg[cfg_index][0], atk_mc2640_init_uxga_cfg[cfg_index][1]);
    }

//    for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_init_svga_cfg)/sizeof(atk_mc2640_init_svga_cfg[0])); cfg_index++)
//    {
//        atk_mc2640_write_reg(atk_mc2640_init_svga_cfg[cfg_index][0], atk_mc2640_init_svga_cfg[cfg_index][1]);
//    }

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    zmow = _atk_mc2640_read_reg(ATK_MC2640_REG_DSP_ZMOW);
    zmoh = _atk_mc2640_read_reg(ATK_MC2640_REG_DSP_ZMOH);
    zmhh = _atk_mc2640_read_reg(ATK_MC2640_REG_DSP_ZMHH);

    g_atk_mc2640_sta.output.width = ((uint16)zmow | ((zmhh & 0x03) << 8)) << 2;
    g_atk_mc2640_sta.output.height = ((uint16)zmoh | ((zmhh & 0x04) << 6)) << 2;
}

/**
 * @brief       初始化ATK-MC2640模块
 * @param       无
 * @retval      ATK_MC2640_EOK   : ATK-MC2640模块初始化成功
 *              ATK_MC2640_ERROR : 通讯出错，ATK-MC2640模块初始化失败
 *              ATK_MC2640_ENOMEM: 内存不足，ATK-MC2640模块初始化失败
 */
uint8 _atk_mc2640_init(void)
{
    uint16 mid;
    uint16 pid;

    _atk_mc2640_hw_init();           /* ATK-MC2640模块硬件初始化 */
    _atk_mc2640_exit_power_down();   /* ATK-MC2640模块退出掉电模式 */
    _atk_mc2640_hw_reset();          /* ATK-MC2640模块硬件复位 */
    _atk_mc2640_sccb_init();         /* ATK-MC2640 SCCB接口初始化 */
    _atk_mc2640_sw_reset();          /* ATK-MC2640模块软件复位 */

    mid = _atk_mc2640_get_mid();     /* 获取制造商ID */
    if (mid != ATK_MC2640_MID)
    {
        return ATK_MC2640_ERROR;
    }

    pid = _atk_mc2640_get_pid();     /* 获取产品ID */
    if (pid != ATK_MC2640_PID)
    {
        return ATK_MC2640_ERROR;
    }

    _atk_mc2640_init_reg();          /* 初始化ATK-MC2640寄存器配置 */

    return ATK_MC2640_EOK;
}

/**
 * @brief       开启ATK-MC2640模块闪光灯
 * @param       无
 * @retval      无
 */
void _atk_mc2640_led_on(void)
{
    _ATK_MC2640_FLASH(1);
}

/**
 * @brief       关闭ATK-MC2640模块闪光灯
 * @param       无
 * @retval      无
 */
void _atk_mc2640_led_off(void)
{
    _ATK_MC2640_FLASH(0);
}

/**
 * @brief       闪烁ATK-MC2640模块控制LED
 * @param       无
 * @retval      无
 */
void _atk_mc2640_led_enable(void)
{
    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM22, 0x81);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM22, 0x01);
}

/**
 * @brief       设置ATK-MC2640模块灯光模式
 * @param       mode: ATK_MC2640_LIGHT_MOED_AUTO  : Auto
 *                    ATK_MC2640_LIGHT_MOED_SUNNY : Sunny
 *                    ATK_MC2640_LIGHT_MOED_CLOUDY: Cloudy
 *                    ATK_MC2640_LIGHT_MOED_OFFICE: Office
 *                    ATK_MC2640_LIGHT_MOED_HOME  : Home
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块灯光模式成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_light_mode(atk_mc2640_light_mode_t mode)
{
    switch (mode)
    {
        case ATK_MC2640_LIGHT_MODE_AUTO:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0xC7, 0x00);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_SUNNY:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0xC7, 0x40);
            _atk_mc2640_write_reg(0xCC, 0x5E);
            _atk_mc2640_write_reg(0xCD, 0x41);
            _atk_mc2640_write_reg(0xCE, 0x54);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_CLOUDY:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0xC7, 0x40);
            _atk_mc2640_write_reg(0xCC, 0x65);
            _atk_mc2640_write_reg(0xCD, 0x41);
            _atk_mc2640_write_reg(0xCE, 0x4F);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_OFFICE:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0xC7, 0x40);
            _atk_mc2640_write_reg(0xCC, 0x52);
            _atk_mc2640_write_reg(0xCD, 0x41);
            _atk_mc2640_write_reg(0xCE, 0x66);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_HOME:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0xC7, 0x40);
            _atk_mc2640_write_reg(0xCC, 0x42);
            _atk_mc2640_write_reg(0xCD, 0x3F);
            _atk_mc2640_write_reg(0xCE, 0x71);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块色彩饱和度
 * @param       saturation: ATK_MC2640_COLOR_SATURATION_0: +2
 *                          ATK_MC2640_COLOR_SATURATION_1: +1
 *                          ATK_MC2640_COLOR_SATURATION_2: 0
 *                          ATK_MC2640_COLOR_SATURATION_3: -1
 *                          ATK_MC2640_COLOR_SATURATION_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块色彩饱和度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_color_saturation(atk_mc2640_color_saturation_t saturation)
{
    switch (saturation)
    {
        case ATK_MC2640_COLOR_SATURATION_0:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x02);
            _atk_mc2640_write_reg(0x7C, 0x03);
            _atk_mc2640_write_reg(0x7D, 0x68);
            _atk_mc2640_write_reg(0x7D, 0x68);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_1:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x02);
            _atk_mc2640_write_reg(0x7C, 0x03);
            _atk_mc2640_write_reg(0x7D, 0x58);
            _atk_mc2640_write_reg(0x7D, 0x58);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_2:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x02);
            _atk_mc2640_write_reg(0x7C, 0x03);
            _atk_mc2640_write_reg(0x7D, 0x48);
            _atk_mc2640_write_reg(0x7D, 0x48);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_3:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x02);
            _atk_mc2640_write_reg(0x7C, 0x03);
            _atk_mc2640_write_reg(0x7D, 0x38);
            _atk_mc2640_write_reg(0x7D, 0x38);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_4:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x02);
            _atk_mc2640_write_reg(0x7C, 0x03);
            _atk_mc2640_write_reg(0x7D, 0x28);
            _atk_mc2640_write_reg(0x7D, 0x28);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块亮度
 * @param       brightness: ATK_MC2640_BRIGHTNESS_0: +2
 *                          ATK_MC2640_BRIGHTNESS_1: +1
 *                          ATK_MC2640_BRIGHTNESS_2: 0
 *                          ATK_MC2640_BRIGHTNESS_3: -1
 *                          ATK_MC2640_BRIGHTNESS_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块亮度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_brightness(atk_mc2640_brightness_t brightness)
{
    switch (brightness)
    {
        case ATK_MC2640_BRIGHTNESS_0:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x09);
            _atk_mc2640_write_reg(0x7D, 0x40);
            _atk_mc2640_write_reg(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_1:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x09);
            _atk_mc2640_write_reg(0x7D, 0x30);
            _atk_mc2640_write_reg(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_2:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x09);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_3:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x09);
            _atk_mc2640_write_reg(0x7D, 0x10);
            _atk_mc2640_write_reg(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_4:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x09);
            _atk_mc2640_write_reg(0x7D, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x00);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块对比度
 * @param       contrast: ATK_MC2640_CONTRAST_0: +2
 *                        ATK_MC2640_CONTRAST_1: +1
 *                        ATK_MC2640_CONTRAST_2: 0
 *                        ATK_MC2640_CONTRAST_3: -1
 *                        ATK_MC2640_CONTRAST_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块对比度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_contrast(atk_mc2640_contrast_t contrast)
{
    switch (contrast)
    {
        case ATK_MC2640_CONTRAST_0:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x07);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x28);
            _atk_mc2640_write_reg(0x7D, 0x0C);
            _atk_mc2640_write_reg(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_1:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x07);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x24);
            _atk_mc2640_write_reg(0x7D, 0x16);
            _atk_mc2640_write_reg(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_2:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x07);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_3:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x07);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x1C);
            _atk_mc2640_write_reg(0x7D, 0x2A);
            _atk_mc2640_write_reg(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_4:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x04);
            _atk_mc2640_write_reg(0x7C, 0x07);
            _atk_mc2640_write_reg(0x7D, 0x20);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7D, 0x34);
            _atk_mc2640_write_reg(0x7D, 0x06);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块特殊效果
 * @param       contrast: ATK_MC2640_SPECIAL_EFFECT_ANTIQUE    : Antique
 *                        ATK_MC2640_SPECIAL_EFFECT_BLUISH     : Bluish
 *                        ATK_MC2640_SPECIAL_EFFECT_GREENISH   : Greenish
 *                        ATK_MC2640_SPECIAL_EFFECT_REDISH     : Redish
 *                        ATK_MC2640_SPECIAL_EFFECT_BW         : B&W
 *                        ATK_MC2640_SPECIAL_EFFECT_NEGATIVE   : Negative
 *                        ATK_MC2640_SPECIAL_EFFECT_BW_NEGATIVE: B&W Negative
 *                        ATK_MC2640_SPECIAL_EFFECT_NORMAL     : Normal
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块特殊效果成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_special_effect(atk_mc2640_special_effect_t effect)
{
    switch (effect)
    {
        case ATK_MC2640_SPECIAL_EFFECT_ANTIQUE:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x40);
            _atk_mc2640_write_reg(0x7D, 0xA6);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BLUISH:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0xA0);
            _atk_mc2640_write_reg(0x7D, 0x40);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_GREENISH:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x40);
            _atk_mc2640_write_reg(0x7D, 0x40);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_REDISH:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x40);
            _atk_mc2640_write_reg(0x7D, 0xC0);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BW:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x18);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x80);
            _atk_mc2640_write_reg(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_NEGATIVE:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x40);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x80);
            _atk_mc2640_write_reg(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BW_NEGATIVE:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x58);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x80);
            _atk_mc2640_write_reg(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_NORMAL:
        {
            _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            _atk_mc2640_write_reg(0x7C, 0x00);
            _atk_mc2640_write_reg(0x7D, 0x00);
            _atk_mc2640_write_reg(0x7C, 0x05);
            _atk_mc2640_write_reg(0x7D, 0x80);
            _atk_mc2640_write_reg(0x7D, 0x80);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块输出图像格式
 * @param       mode: ATK_MC2640_OUTPUT_FORMAT_RGB565: RGB565
 *                    ATK_MC2640_OUTPUT_FORMAT_JPEG : JPEG
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出图像格式成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_output_format(atk_mc2640_output_format_t format)
{
    uint32 cfg_index;

    switch (format)
    {
        case ATK_MC2640_OUTPUT_FORMAT_RGB565:
        {
            for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_set_rgb565_cfg)/sizeof(atk_mc2640_set_rgb565_cfg[0])); cfg_index++)
            {
                _atk_mc2640_write_reg(atk_mc2640_set_rgb565_cfg[cfg_index][0], atk_mc2640_set_rgb565_cfg[cfg_index][1]);
            }
            break;
        }
        case ATK_MC2640_OUTPUT_FORMAT_JPEG:
        {
            for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_set_yuv422_cfg)/sizeof(atk_mc2640_set_yuv422_cfg[0])); cfg_index++)
            {
                _atk_mc2640_write_reg(atk_mc2640_set_yuv422_cfg[cfg_index][0], atk_mc2640_set_yuv422_cfg[cfg_index][1]);
            }
            for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_set_jpeg_cfg)/sizeof(atk_mc2640_set_jpeg_cfg[0])); cfg_index++)
            {
                _atk_mc2640_write_reg(atk_mc2640_set_jpeg_cfg[cfg_index][0], atk_mc2640_set_jpeg_cfg[cfg_index][1]);
            }
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块输出图像分辨率
 * @param       width : 输出图像宽度，必须是4的倍数
 *              height: 输出图像高度，必须是4的倍数
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出图像大小成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 *              ATK_MC2640_ENOMEM: 内存不足
 */
uint8 _atk_mc2640_set_output_size(uint16 width, uint16 height)
{
    uint16 output_width;
    uint16 output_height;

    if (((width & (4 - 1)) != 0) || ((height & (4 - 1)) != 0))
    {
        return ATK_MC2640_EINVAL;
    }

    g_atk_mc2640_sta.output.width = width;
    g_atk_mc2640_sta.output.height = height;

    output_width = width >> 2;
    output_height = height >> 2;

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x04);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_ZMOW, (uint8)(output_width & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_ZMOH, (uint8)(output_height & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_ZMHH, ((uint8)(output_width >> 8) & 0x03) | ((uint8)(output_height >> 6) & 0x04));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x00);

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块传感器窗口
 * @param       start_x: 传感器窗口起始X坐标
 *              start_y: 传感器窗口起始Y坐标
 *              width  : 传感器窗口宽度
 *              height : 传感器窗口高度
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出图像大小成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_sensor_window(uint16 start_x, uint16 start_y, uint16 width, uint16 height)
{
    uint16 end_x;
    uint16 end_y;
    uint8 raw_com1;
    uint8 com1;
    uint8 raw_reg32;
    uint8 reg32;

    end_x = start_x + (width >> 1);
    end_y = start_y + (height >> 1);

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);

    raw_com1 = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_COM1);
    com1 = (raw_com1 & 0xF0) | (((end_y & 0x03) << 2) | (start_y & 0x03));
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM1, com1);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_VSTRT, start_y >> 2);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_VEND, end_y >> 2);

    raw_reg32 = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_REG32);
    reg32 = (raw_reg32 & 0xC0) | (((end_x & 0x07) << 3) | (start_x & 0x07));
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_REG32, reg32);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_HREFST, start_x >> 3);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_HREFEND, end_x >> 3);

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块输出图像窗口
 * @param       off_x : 输出图像窗口偏移X坐标
 *              off_y : 输出图像窗口偏移Y坐标
 *              width : 输出图像窗口宽度，必须是4的倍数
 *              height: 输出图像窗口高度，必须是4的倍数
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出图像大小成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_image_window(uint16 off_x, uint16 off_y, uint16 width, uint16 height)
{
    uint16 hsize;
    uint16 vsize;
    uint8 vhyx;

    if (((width & (4 - 1)) != 0) || ((height & (4 - 1)) != 0))
    {
        return ATK_MC2640_EINVAL;
    }

    hsize = width >> 2;
    vsize = height >> 2;

    vhyx = (uint8)(((vsize >> 1) & 0x80) | ((off_y >> 4) & 0x70) | ((hsize >> 5) & 0x08) | ((off_x >> 8) & 0x07));

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x04);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_HSIZE, (uint8)(hsize & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_VSIZE, (uint8)(vsize & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_XOFFL, (uint8)(off_x & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_YOFFL, (uint8)(off_y & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_VHYX, vhyx);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_TEST, (uint8)((hsize >> 2) & 0x80));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x00);

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块输出图像大小
 * @param       off_x : 输出图像窗口偏移X坐标
 *              off_y : 输出图像窗口偏移Y坐标
 *              width : 输出图像窗口宽度，必须是4的倍数
 *              height: 输出图像窗口高度，必须是4的倍数
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出图像大小成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
void _atk_mc2640_set_image_size(uint16 width, uint16 height)
{
    uint8 sizel;

    sizel = (uint8)(((width & 0x07) << 3) | (height & 0x07) | ((width >> 4) & 0x80));

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x04);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_HSIZE8, (uint8)((width >> 3) & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_VSIZE8, (uint8)((height >> 3) & 0x00FF));
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_SIZEL, sizel);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_RESET, 0x00);
}

/**
 * @brief       设置ATK-MC2640模块纵向和横向跳采样系数
 * @param       V_DIVIDER : 纵向跳采样系数（对本程序2比较合适）
 *              H_DIVIDER : 横向跳采样系数（对本程序2比较合适）
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块跳采样系数成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_divider(uint8 V_DIVIDER, uint8 H_DIVIDER)
{
    _atk_mc2640_write_reg(0x50, ((V_DIVIDER << 3) & 0x38) + (H_DIVIDER & 0x07));
    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块纵向和横向翻转
 * @param       Vertical_flip : 纵向是否翻转
 *              Horizontal_mirror : 横向是否翻转
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块跳采样系数成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_flip(uint8 Vertical_flip, uint8 Horizontal_mirror)
{
    uint8 temp = _atk_mc2640_read_reg(0x04);
    temp |= (Horizontal_mirror << 7) + (Vertical_flip << 6);
    _atk_mc2640_write_reg(0xff, 0x01);
    _atk_mc2640_write_reg(0x04, temp);
    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块输出速率
 * @param       clk_dev : Clock divider（0~63）
 *              pclk_dev: DVP PCLK（1~127）
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块输出速率成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8 _atk_mc2640_set_output_speed(uint8 clk_dev, uint8 pclk_dev)
{
    if (clk_dev > 63)
    {
        return ATK_MC2640_EINVAL;
    }

    if ((pclk_dev == 0) || (pclk_dev > 127))
    {
        return ATK_MC2640_EINVAL;
    }

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    _atk_mc2640_write_reg(ATK_MC2640_REG_DSP_R_DVP_SP, pclk_dev);
    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_CLKRC, clk_dev);

    return ATK_MC2640_EOK;
}

/**
 * @brief       开启ATK-MC2640模块彩条测试
 * @param       无
 * @retval      无
 */
void _atk_mc2640_colorbar_enable(void)
{
    uint8 com7;

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    com7 = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_COM7);
    com7 |= (uint8)(1 << 1);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM7, com7);
}

/**
 * @brief       关闭ATK-MC2640模块彩条测试
 * @param       无
 * @retval      无
 */
void _atk_mc2640_colorbar_disable(void)
{
    uint8 com7;

    _atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    com7 = _atk_mc2640_read_reg(ATK_MC2640_REG_SENSOR_COM7);
    com7 &= ~(uint8)(1 << 1);
    _atk_mc2640_write_reg(ATK_MC2640_REG_SENSOR_COM7, com7);
}

/**
 * @brief       帧中断函数
 * @param       无
 * @retval      无
 */
void IO_Vsync_ISR(void) {
    if (IfxPort_getPinState(CAM_VSYNC_SW_EXTI_PIN.pin.port, CAM_VSYNC_SW_EXTI_PIN.pin.pinIndex) == 1) {
        IfxDma_setChannelDestinationAddress(&MODULE_DMA, CAMERA_PCLK_PRIORITY,
        (void*) IFXCPU_GLB_ADDR_DSPR(IfxCpu_getCoreId(), writing_img_ptr));
    } else {
        IfxDma_disableChannelTransaction(&MODULE_DMA, CAMERA_PCLK_PRIORITY);
        _Camera_Image_Received();
    }
    IfxPort_togglePin(IO_LED2_PORT, IO_LED2_PIN);
}

/**
 * @brief       行中断函数
 * @param       无
 * @retval      无
 */
void IO_Hsync_ISR(void) {
    if (IfxPort_getPinState(CAM_HSYNC_SW_EXTI_PIN.pin.port, CAM_HSYNC_SW_EXTI_PIN.pin.pinIndex) == 1) {
        IfxDma_enableChannelTransaction(&MODULE_DMA, CAMERA_PCLK_PRIORITY);
    } else {
        IfxDma_disableChannelTransaction(&MODULE_DMA, CAMERA_PCLK_PRIORITY);
    }
    IfxPort_togglePin(IO_LED3_PORT, IO_LED3_PIN);
}


// P15.04 VSYNC channelId = 0
// P11.10 PCLK channelId = 6
// P10.03 HREF channelId = 3

// EXI flags
const unsigned char PinIrqVectabNum[4] =
        {CAMERA_VSYNC_TOS, 0, CAMERA_PCLK_TOS, CAMERA_HSYNC_TOS};

// EXI priority
const unsigned char PinIrqPriority[4] = {CAMERA_VSYNC_PRIORITY, 0, CAMERA_PCLK_PRIORITY, CAMERA_HSYNC_PRIORITY};

/**
 * @brief       外部中断引脚设置
 * @param       port : 中断脚port
 *              pin  : 中断脚pin
 *              mode : 中断触发边沿配置
 * @retval      无
 */
void _PIN_Exti (Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    int i, j;

    // disable interrupts
    unsigned char interruptState = IfxCpu_disableInterrupts();

    IfxScu_Req_In *reqPin = 0;

    /// check pins
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (IfxScu_Req_In_pinTable[0][i][j] == NULL_PTR)
            {

            }
            else if ((uint32) port == (uint32) IfxScu_Req_In_pinTable[0][i][j]->pin.port
                    && pinIndex == IfxScu_Req_In_pinTable[0][i][j]->pin.pinIndex)
            {
                reqPin = IfxScu_Req_In_pinTable[0][i][j];
            }
        }
    }

    if (!reqPin)
    {
        while (1)
            ;
    }

    // pull up/down
    IfxScuEru_initReqPin(reqPin, mode);

    // set channel
    IfxScuEru_InputChannel inputChannel = (IfxScuEru_InputChannel) reqPin->channelId;

    // set up interrupt mode
    IfxScuEru_disableFallingEdgeDetection(inputChannel);
    IfxScuEru_disableRisingEdgeDetection(inputChannel);
    if (mode == IfxPort_InputMode_pullUp || mode == IfxPort_InputMode_noPullDevice)
    {
        IfxScuEru_enableFallingEdgeDetection(inputChannel);
    }
    if (mode == IfxPort_InputMode_pullDown || mode == IfxPort_InputMode_noPullDevice)
    {
        IfxScuEru_enableRisingEdgeDetection(inputChannel);
    }
    IfxScuEru_enableAutoClear(inputChannel);

    // signal destination
    IfxScuEru_InputNodePointer triggerSelect = (IfxScuEru_InputNodePointer) inputChannel;
    IfxScuEru_OutputChannel outputChannel = (IfxScuEru_OutputChannel) inputChannel;

    // connecting matrix
    IfxScuEru_enableTriggerPulse(inputChannel);
    IfxScuEru_connectTrigger(inputChannel, triggerSelect);

    // output channels
    IfxScuEru_setFlagPatternDetection(outputChannel, inputChannel, 0);
    IfxScuEru_disablePatternDetectionTrigger(outputChannel);
    IfxScuEru_setInterruptGatingPattern(outputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);

    uint8 vectabNum = PinIrqVectabNum[(int) inputChannel % 4];
    uint8 Priority = PinIrqPriority[(int) inputChannel % 4];

    // service req config
    {
        volatile Ifx_SRC_SRCR *src = &MODULE_SRC.SCU.SCUERU[(int) outputChannel % 4];
        IfxSrc_init(src, vectabNum, Priority);
        IfxSrc_enable(src);
    }

    // enable interrupts
    IfxCpu_restoreInterrupts(interruptState);
}

/**
 * 摄像头 DMA 配置结构体
 */
typedef struct
{
        Ifx_DMA_CH linkedList[2];               //总是报错，去掉了IFX_ALIGN(256)，实测没有影响
        //IFX_ALIGN(256) Ifx_DMA_CH linkedList[2]; //链表存储。必须与256位地址对齐，否则DMA无法读取
        IfxDma_Dma_Channel chn;                 // Dma channel handle
} Dma_Camera_t;

Dma_Camera_t g_DmaCameraLinkedList;

void _DMA_CameraInitConfig (unsigned long srcStartAddr, unsigned long dstStartAddr, unsigned long channel)
{
    /* 关闭中断 */
    boolean interruptState = IfxCpu_disableInterrupts();

    /* create module config */
    IfxDma_Dma_Config dmaConfig;
    IfxDma_Dma_initModuleConfig(&dmaConfig, &MODULE_DMA);

    /* initialize module */
    IfxDma_Dma dma;
    IfxDma_Dma_initModule(&dma, &dmaConfig);

    /* initial configuration for all channels */
    IfxDma_Dma_ChannelConfig cfg;
    IfxDma_Dma_initChannelConfig(&cfg, &dma);

//  cfg.busPriority   = IfxDma_ChannelBusPriority_high;                    //优先级最高
    cfg.requestMode = IfxDma_ChannelRequestMode_oneTransferPerRequest;   //请求启动一次传输
    cfg.moveSize = IfxDma_ChannelMoveSize_8bit;                       //一次move 8bit
    cfg.shadowControl = IfxDma_ChannelShadow_none;                         //单次模式
    cfg.operationMode = IfxDma_ChannelOperationMode_continuous;            //传输完成继续传输
    cfg.hardwareRequestEnabled = TRUE;                                     //使能硬件触发传输
    cfg.sourceAddress = IFXCPU_GLB_ADDR_DSPR(IfxCpu_getCoreId(), srcStartAddr);  //设置源地址
    cfg.sourceCircularBufferEnabled = TRUE;                                      //保持源地址不变
    cfg.sourceAddressCircularRange = IfxDma_ChannelIncrementCircular_none;
    cfg.channelId = channel;                                                 //设置DMA通道
    cfg.channelInterruptEnabled = FALSE;
    cfg.channelInterruptPriority = CAMERA_DMA_CPLT_PRIORITY;
    cfg.channelInterruptTypeOfService = CAMERA_DMA_TOS;

    /* 设置传输表述符 */
    cfg.destinationAddress = IFXCPU_GLB_ADDR_DSPR(IfxCpu_getCoreId(), dstStartAddr);

    /* address to next transaction set */
    cfg.shadowAddress = 0;

    cfg.transferCount = 11280;
    cfg.transferCount = CAM_IMAGE_HEIGHT * CAM_IMAGE_WIDTH * 2;

    /* transfer first transaction set into DMA channel */
    IfxDma_Dma_initChannel(&g_DmaCameraLinkedList.chn, &cfg);

    /* transfer into linked list storage */
    IfxDma_Dma_initLinkedListEntry((void*) &g_DmaCameraLinkedList.linkedList[0], &cfg);

    /* clear service request flag */
    (IfxDma_Dma_getSrcPointer(&g_DmaCameraLinkedList.chn))->B.CLRR = 1;

    IfxDma_clearChannelInterrupt(&MODULE_DMA, channel);

    IfxDma_disableChannelTransaction(&MODULE_DMA, channel);

    /* 开启中断 */
    IfxCpu_restoreInterrupts(interruptState);
}




/**
 * @brief       ATK-MC2640初始化，可以在main函数中调用
 * @param       无
 * @retval      0：失败；1：成功
 */
uint8 Camera_Init(void) {

    uint8 ret;
    ret  = _atk_mc2640_init();                                               /* 初始化ATK-MC2640模块 */
    ret |= _atk_mc2640_set_output_format(ATK_MC2640_OUTPUT_FORMAT_RGB565);   /* 输出图像格式 */
    ret |= _atk_mc2640_set_output_size(CAM_IMAGE_WIDTH, CAM_IMAGE_HEIGHT);         /* 输出图像分辨率 */
    ret |= _atk_mc2640_set_image_window(100, 0, CAM_IMAGE_WIDTH * 8, CAM_IMAGE_HEIGHT * 8);
    ret |= _atk_mc2640_set_divider(2, 2);
    ret |= _atk_mc2640_set_sensor_window(200, 0, CAM_IMAGE_WIDTH * 8, CAM_IMAGE_HEIGHT * 8);
    ret |= _atk_mc2640_set_flip(1, 0);
    if (ret != 0) {
        return 0;
        // printf("ATK-MC2640 Init Failed!\r\n");
//        while (1) {
//
//        }
    }
    _atk_mc2640_set_output_speed(2, 22);                                     /* 输出速率 */
    _atk_mc2640_set_light_mode(ATK_MC2640_LIGHT_MODE_SUNNY);                 /* 设置灯光模式 */
    _atk_mc2640_set_color_saturation(ATK_MC2640_COLOR_SATURATION_1);         /* 设置色彩饱和度 */
    _atk_mc2640_set_brightness(ATK_MC2640_BRIGHTNESS_1);                     /* 设置亮度 */
    _atk_mc2640_set_contrast(ATK_MC2640_CONTRAST_2);                         /* 设置对比度 */
    _atk_mc2640_set_special_effect(ATK_MC2640_SPECIAL_EFFECT_NORMAL);        /* 设置特殊效果 */


    IfxCpu_disableInterrupts();

    _PIN_Exti(CAM_PCLK_SW_EXTI_PIN.pin.port, CAM_PCLK_SW_EXTI_PIN.pin.pinIndex, PIN_IRQ_MODE_RISING);
    _PIN_Exti(CAM_VSYNC_SW_EXTI_PIN.pin.port, CAM_VSYNC_SW_EXTI_PIN.pin.pinIndex, PIN_IRQ_MODE_RISING_FALLING);
    _PIN_Exti(CAM_HSYNC_SW_EXTI_PIN.pin.port, CAM_HSYNC_SW_EXTI_PIN.pin.pinIndex, PIN_IRQ_MODE_RISING_FALLING);

    _DMA_CameraInitConfig((unsigned long) (&(MODULE_P02.IN.U)), (unsigned long) writing_img_ptr, CAMERA_PCLK_PRIORITY);

    IfxCpu_enableInterrupts();

    return 1;
}


