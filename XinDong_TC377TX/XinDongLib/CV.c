#include "CV.h"
#include <string.h>
#include <math.h>

//	reference code: none, good luck
//	reference code: TC264_XinDong_Demo_v42/Cpu1_Main.c

// 全局变量
sint16 g_buxian = 0;  // 补线方向判断变量

// 图像预处理：自适应阈值（每行滑动窗口均值） - 一维展开
void CV_PreprocessImage(uint16 *input_img, 
                       uint16 *mask) {
    uint16 y, x;
    const uint16 threshold = 200; // 手动阈值，可按需调整
    for (y = 0; y < CV_IMAGE_HEIGHT; y++) {
        for (x = 0; x < CV_IMAGE_WIDTH; x++) {
            if (input_img[y * CV_IMAGE_WIDTH + x] > threshold) {
                mask[y * CV_IMAGE_WIDTH + x] = 255;
            } else {
                mask[y * CV_IMAGE_WIDTH + x] = 0;
            }
        }
    }
}

// 检查指定区域是否全为黑色
uint16 CV_IsRegionEmpty(uint16 *mask, 
                      uint16 y, uint16 start_x, uint16 end_x) {
    uint16 x;
    for (x = start_x; x < end_x; x++) {
        if (mask[y * CV_IMAGE_WIDTH + x] == 255) {
            return 0;  // 不是全空
        }
    }
    return 1;  // 全空
}

// 计算指定区域的平均位置
uint16 CV_CalculateAveragePosition(uint16 *mask, 
                                  uint16 y, uint16 start_x, uint16 end_x) {
    uint16 x;
    uint32 sum = 0;
    uint16 count = 0;
    
    for (x = start_x; x < end_x; x++) {
        if (mask[y * CV_IMAGE_WIDTH + x] == 255) {
            sum += x;
            count++;
        }
    }
    if (count == 0) {
        return start_x;  // 如果没有找到白色像素，返回起始位置
    }
    return (uint16)(sum / count);
}

// 从左到右查找第一个白色像素，若未找到，返回start_x
uint16 CV_FindFirstWhiteFromLeft(uint16 *mask,
                               uint16 y, uint16 start_x, uint16 end_x) {
    uint16 x;
    for (x = start_x; x < end_x; x++) {
        if (mask[y * CV_IMAGE_WIDTH + x] == 255) {
            return x;
        }
    }
    return start_x;
}

// 从右到左查找第一个白色像素，若未找到，返回end_x
uint16 CV_FindFirstWhiteFromRight(uint16 *mask,
                                uint16 y, uint16 start_x, uint16 end_x) {
    sint32 x; // 需要有符号类型以便向下计数
    for (x = (sint32)end_x - 1; x >= (sint32)start_x; x--) {
        if (mask[y * CV_IMAGE_WIDTH + (uint16)x] == 255) {
            return (uint16)x;
        }
    }
    return end_x;
}

// 计算图像中线的偏差（优化版本，直接在原图上处理）- 一维展开
sint16 CV_CalculateMidlineError(uint16 *input_img, 
                              uint16 *mask) {
    sint16 y;
    uint16 half_width = CV_IMAGE_WIDTH / 2;
    uint16 half = half_width;  // 从下往上扫描赛道，最下端取图片中线为分割线
    uint16 left, right, mid;
    sint16 mid_output = 0;
    g_buxian = 0;  // 重置补线方向判断变量
    
    // 从下往上扫描
    for (y = CV_IMAGE_HEIGHT - 1; y >= 0; y--) {
        // 按要求：左端从左往右找第一个白点，右端从右往左找第一个白点
        // 左侧范围：[0, half)
        if (CV_IsRegionEmpty(mask, y, 0, half)) {
            left = 0;
            if (y >= CV_SCAN_START_Y && y <= CV_SCAN_END_Y) {
                g_buxian -= 1;  // 左边补线
            }
        } else {
            left = CV_FindFirstWhiteFromLeft(mask, y, 0, half);
        }

        // 右侧范围：[half, CV_IMAGE_WIDTH)
        if (CV_IsRegionEmpty(mask, y, half, CV_IMAGE_WIDTH)) {
            right = CV_IMAGE_WIDTH;
            if (y >= CV_SCAN_START_Y && y <= CV_SCAN_END_Y) {
                g_buxian += 1;  // 右边补线
            }
        } else {
            right = CV_FindFirstWhiteFromRight(mask, y, half, CV_IMAGE_WIDTH);
        }
        
        // 计算拟合中点
        mid = (left + right) / 2;
        half = mid;  // 递归，从下往上确定分割线
        
        // 在原图上画出拟合中线（可选，用于调试）
        if (mid < CV_IMAGE_WIDTH) {
            input_img[y * CV_IMAGE_WIDTH + mid] = 255;
        }
        
        // 在指定Y位置提取中点
        if (y == CV_TARGET_Y) {
            mid_output = mid;
        }
    }
    
    // 计算图片中点与指定提取中点的误差
    // error为正数右转，为负数左转
    return (CV_IMAGE_WIDTH / 2) - mid_output;
}

// 中线检测主函数（优化版本，最多使用2个buffer）- 输入一维展开
CV_Result_t CV_DetectMidline(uint16 *input_img) {
    CV_Result_t result;
    static uint16 mask[CV_IMAGE_HEIGHT * CV_IMAGE_WIDTH];
    
    // 参数检查
    if (input_img == NULL) {
        result.valid = 0;
        result.error = 0;
        result.image_data = NULL;
        return result;
    }
    
    // 初始化结果
    result.valid = 0;
    result.error = 0;
    result.image_data = input_img;
    
    // 清空掩码缓冲区
    memset(mask, 0, sizeof(mask));
    
    // 图像预处理：生成二值化掩码
    CV_PreprocessImage(input_img, mask);
    
    // 直接在原图上计算中线偏差（节省一个buffer）
    result.error = CV_CalculateMidlineError(input_img, mask);
    result.valid = 1;
    
    return result;
}

// 图像处理主函数
CV_Result_t CV_ProcessImage(void) {
    CV_Result_t result;
    uint16 *img_ptr;
    
    // 初始化结果
    result.valid = 0;
    result.error = 0;
    result.image_data = NULL;
    
    // 获取最新的图像缓冲区
    img_ptr = (uint16*)Camera_GetLatest();
    
    if (img_ptr != NULL) {
        // 进行中线检测
        result = CV_DetectMidline(img_ptr);
        // 释放图像缓冲区 - 确保在任何情况下都释放（显式转换以避免不同类型指针告警）
        Camera_Release((uint16 (*)[CAM_IMAGE_WIDTH])img_ptr);
    } else {
        // 如果没有获取到图像缓冲区，返回无效结果
        result.valid = 0;
        result.error = 0;
        result.image_data = NULL;
    }
    
    return result;
}

// ================== 行/列投影法 ==================
// 统计每一行的白色像素数量，结果存入row_proj，长度为CV_IMAGE_HEIGHT
void CV_RowProjection(uint16 *mask, uint16 row_proj[CV_IMAGE_HEIGHT]) {
    uint16 y, x;
    uint16 count;
    for (y = 0; y < CV_IMAGE_HEIGHT; y++) {
        count = 0;
        for (x = 0; x < CV_IMAGE_WIDTH; x++) {
            if (mask[y * CV_IMAGE_WIDTH + x] == 255) {
                count++;
            }
        }
        row_proj[y] = count;
    }
}

// 统计每一列的白色像素数量，结果存入col_proj，长度为CV_IMAGE_WIDTH
void CV_ColProjection(uint16 *mask, uint16 col_proj[CV_IMAGE_WIDTH]) {
    uint16 y, x;
    uint16 count;
    for (x = 0; x < CV_IMAGE_WIDTH; x++) {
        count = 0;
        for (y = 0; y < CV_IMAGE_HEIGHT; y++) {
            if (mask[y * CV_IMAGE_WIDTH + x] == 255) {
                count++;
            }
        }
        col_proj[x] = count;
    }
}

// ================== 简单形态学操作 ==================
// 3x3腐蚀（Erosion）：去除噪点，细化赛道
void CV_Erode3x3(uint16 *src, uint16 *dst) {
    uint16 y, x, i, j;
    for (y = 1; y < CV_IMAGE_HEIGHT - 1; y++) {
        for (x = 1; x < CV_IMAGE_WIDTH - 1; x++) {
            uint8 flag = 1;
            for (i = y - 1; i <= y + 1; i++) {
                for (j = x - 1; j <= x + 1; j++) {
                    if (src[i * CV_IMAGE_WIDTH + j] == 0) {
                        flag = 0;
                        break;
                    }
                }
                if (!flag) break;
            }
            dst[y * CV_IMAGE_WIDTH + x] = flag ? 255 : 0;
        }
    }
    // 边界直接赋值为0
    for (y = 0; y < CV_IMAGE_HEIGHT; y++) {
        dst[y * CV_IMAGE_WIDTH + 0] = 0;
        dst[y * CV_IMAGE_WIDTH + (CV_IMAGE_WIDTH-1)] = 0;
    }
    for (x = 0; x < CV_IMAGE_WIDTH; x++) {
        dst[0 * CV_IMAGE_WIDTH + x] = 0;
        dst[(CV_IMAGE_HEIGHT-1) * CV_IMAGE_WIDTH + x] = 0;
    }
}

// 3x3膨胀（Dilation）：填补小空洞，粗化赛道
void CV_Dilate3x3(uint16 *src, uint16 *dst) {
    uint16 y, x, i, j;
    for (y = 1; y < CV_IMAGE_HEIGHT - 1; y++) {
        for (x = 1; x < CV_IMAGE_WIDTH - 1; x++) {
            uint8 flag = 0;
            for (i = y - 1; i <= y + 1; i++) {
                for (j = x - 1; j <= x + 1; j++) {
                    if (src[i * CV_IMAGE_WIDTH + j] == 255) {
                        flag = 1;
                        break;
                    }
                }
                if (flag) break;
            }
            dst[y * CV_IMAGE_WIDTH + x] = flag ? 255 : 0;
        }
    }
    // 边界直接赋值为0
    for (y = 0; y < CV_IMAGE_HEIGHT; y++) {
        dst[y * CV_IMAGE_WIDTH + 0] = 0;
        dst[y * CV_IMAGE_WIDTH + (CV_IMAGE_WIDTH-1)] = 0;
    }
    for (x = 0; x < CV_IMAGE_WIDTH; x++) {
        dst[0 * CV_IMAGE_WIDTH + x] = 0;
        dst[(CV_IMAGE_HEIGHT-1) * CV_IMAGE_WIDTH + x] = 0;
    }
}

/*
用法示例：
    static uint16 mask[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
    static uint16 temp[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
    // 先用CV_PreprocessImage生成mask
    CV_PreprocessImage(input_img, &mask);
    // 腐蚀
    CV_Erode3x3(&mask, &temp);
    // 膨胀
    CV_Dilate3x3(&mask, &temp);
    // temp为输出结果
*/

// ================== 斜率/角度检测 ==================
// 计算中线在多行的横坐标，返回斜率和角度（单位：度）
// rows: 需要检测的行号数组，num_rows: 数组长度
// mid_x: 输出每一行的中线横坐标
// 返回值：角度（float，单位度），斜率通过指针返回
float CV_CalcMidlineSlopeAngle(uint16 *mask, const uint16* rows, uint16 num_rows, uint16 mid_x[], float* out_slope) {
    uint16 i;
    // 计算每一行的中线横坐标
    for (i = 0; i < num_rows; i++) {
        mid_x[i] = CV_CalculateAveragePosition(mask, rows[i], 0, CV_IMAGE_WIDTH);
    }
    // 用首尾两点拟合直线，计算斜率
    uint16 dy = rows[num_rows-1] - rows[0];
    uint16 dx = mid_x[num_rows-1] - mid_x[0];
    float slope = (dy != 0) ? ((float)dx / (float)dy) : 0.0f;
    if (out_slope) *out_slope = slope;
    // 角度 = atan(斜率) * 180 / pi
    float angle = atan(slope) * 57.29578f;
    return angle;
}

/*
用法示例：
    static uint16 mask[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
    uint16 rows[5] = {100, 110, 120, 130, 140}; // 选取5个感兴趣的行
    uint16 mid_x[5];
    float slope, angle;
    // 先用CV_PreprocessImage生成mask
    CV_PreprocessImage(input_img, &mask);
    // 计算中线斜率和角度
    angle = CV_CalcMidlineSlopeAngle(&mask, rows, 5, mid_x, &slope);
    // angle为角度，slope为斜率
*/


/*
 * 使用示例：
 * 
 * 在主程序循环中调用：
 * 
 * CV_Result_t cv_result;
 * 
 * // 处理图像并获取中线偏差
 * cv_result = CV_ProcessImage();
 * 
 * if (cv_result.valid) {
 *     // 获取偏差值
 *     sint16 error = cv_result.error;
 *     
 *     // 获取补线方向
 *     sint16 buxian = g_buxian;
 *     
 *     // 根据偏差控制转向
 *     if (error > 0) {
 *         // 右转
 *         // 执行右转控制代码
 *     } else if (error < 0) {
 *         // 左转
 *         // 执行左转控制代码
 *     }
 *     
 *     // 根据补线方向进行额外处理
 *     if (buxian > 0) {
 *         // 右边补线，可能需要特殊处理
 *     } else if (buxian < 0) {
 *         // 左边补线，可能需要特殊处理
 *     }
 * }
 * 
 * 内存优化说明：
 * - 当前实现最多使用2个图像缓冲区：
 *   1. input_img: 来自Camera模块的原始图像
 *   2. mask: 二值化掩码缓冲区（静态分配）
 * - 移除了follow缓冲区，直接在原图上处理
 * - 总内存使用：约90KB (45.12KB × 2) - 注意：由于使用uint16，内存使用量翻倍
 * 
 * 注意：CV_ProcessImage()函数会自动处理图像缓冲区的获取和释放，
 * 无需手动调用Camera_Release()函数。
 * 
 * 如果需要手动管理图像缓冲区，可以使用以下方式：
 * 
 * uint16 (*img_ptr)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
 * CV_Result_t result;
 * 
 * // 获取图像缓冲区
 * img_ptr = (uint16 (*)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH])Camera_GetLatest();
 * 
 * if (img_ptr != NULL) {
 *     // 进行图像处理
 *     result = CV_DetectMidline(img_ptr);
 *     
 *     // 处理结果...
 *     
 *     // 重要：处理完成后必须释放缓冲区
 *     Camera_Release(img_ptr);
 * }
 */

// ================== 中线左右两侧颜色检测 ==================
// 获取中线左右两侧的颜色值
// input_img: 输入图像
// y_position: 检测的Y坐标位置
// left_range: 左侧检测范围（像素数）
// right_range: 右侧检测范围（像素数）
// left_color: 输出左侧平均颜色值
// right_color: 输出右侧平均颜色值
void CV_GetMidlineSideColors(uint16 *input_img, 
                            uint16 y_position, uint16 left_range, uint16 right_range,
                            uint16* left_color, uint16* right_color) {
    uint16 x;
    uint16 center_x = CV_IMAGE_WIDTH / 2;  // 图像中心X坐标
    uint16 left_start, left_end, right_start, right_end;
    uint32 left_sum = 0, right_sum = 0;
    uint16 left_count = 0, right_count = 0;
    
    // 参数检查
    if (input_img == NULL || left_color == NULL || right_color == NULL) {
        return;
    }
    
    // 检查Y坐标是否有效
    if (y_position >= CV_IMAGE_HEIGHT) {
        return;
    }
    
    // 计算左侧检测范围
    left_start = (center_x > left_range) ? (center_x - left_range) : 0;
    left_end = center_x;
    
    // 计算右侧检测范围
    right_start = center_x;
    right_end = (center_x + right_range < CV_IMAGE_WIDTH) ? (center_x + right_range) : CV_IMAGE_WIDTH;
    
    // 计算左侧平均颜色值
    for (x = left_start; x < left_end; x++) {
        left_sum += input_img[y_position * CV_IMAGE_WIDTH + x];
        left_count++;
    }
    
    // 计算右侧平均颜色值
    for (x = right_start; x < right_end; x++) {
        right_sum += input_img[y_position * CV_IMAGE_WIDTH + x];
        right_count++;
    }
    
    // 计算平均值并输出
    if (left_count > 0) {
        *left_color = (uint16)(left_sum / left_count);
    } else {
        *left_color = 0;
    }
    
    if (right_count > 0) {
        *right_color = (uint16)(right_sum / right_count);
    } else {
        *right_color = 0;
    }
}

/*
 * 使用示例：
 * 
 * uint16 (*img_ptr)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
 * uint16 left_color, right_color;
 * 
 * // 获取图像缓冲区
 * img_ptr = (uint16 (*)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH])Camera_GetLatest();
 * 
 * if (img_ptr != NULL) {
 *     // 获取中线左右两侧的颜色（在Y=100位置，左右各检测10个像素）
 *     CV_GetMidlineSideColors(img_ptr, 100, 10, 10, &left_color, &right_color);
 *     
 *     // 使用颜色值进行判断
 *     if (left_color > right_color) {
 *         // 左侧颜色较亮
 *     } else if (right_color > left_color) {
 *         // 右侧颜色较亮
 *     } else {
 *         // 两侧颜色相近
 *     }
 *     
 *     // 释放图像缓冲区
 *     Camera_Release(img_ptr);
 * }
 */

// 获取动态中线左右两侧的颜色值（基于检测到的中线位置）
// input_img: 输入图像
// mask: 二值化掩码图像
// y_position: 检测的Y坐标位置
// left_range: 左侧检测范围（像素数）
// right_range: 右侧检测范围（像素数）
// left_color: 输出左侧平均颜色值
// right_color: 输出右侧平均颜色值
// midline_x: 输出检测到的中线X坐标

void CV_GetDynamicMidlineSideColors(uint16 *input_img, 
                                   uint16 *mask,
                                   uint16 y_position, uint16 left_range, uint16 right_range,
                                   uint16* left_color, uint16* right_color, uint16* midline_x) {
    uint16 x;
    uint16 detected_midline_x;
    uint16 left_start, left_end, right_start, right_end;
    uint32 left_sum = 0, right_sum = 0;
    uint16 left_count = 0, right_count = 0;
    
    // 参数检查
    if (input_img == NULL || mask == NULL || left_color == NULL || right_color == NULL || midline_x == NULL) {
        return;
    }
    
    // 检查Y坐标是否有效
    if (y_position >= CV_IMAGE_HEIGHT) {
        return;
    }
    
    // 检测指定Y位置的中线X坐标
    detected_midline_x = CV_CalculateAveragePosition(mask, y_position, 0, CV_IMAGE_WIDTH);
    *midline_x = detected_midline_x;
    
    // 计算左侧检测范围
    left_start = (detected_midline_x > left_range) ? (detected_midline_x - left_range) : 0;
    left_end = detected_midline_x;
    
    // 计算右侧检测范围
    right_start = detected_midline_x;
    right_end = (detected_midline_x + right_range < CV_IMAGE_WIDTH) ? (detected_midline_x + right_range) : CV_IMAGE_WIDTH;
    
    // 计算左侧平均颜色值
    for (x = left_start; x < left_end; x++) {
        left_sum += input_img[y_position * CV_IMAGE_WIDTH + x];
        left_count++;
    }
    
    // 计算右侧平均颜色值
    for (x = right_start; x < right_end; x++) {
        right_sum += input_img[y_position * CV_IMAGE_WIDTH + x];
        right_count++;
    }
    
    // 计算平均值并输出
    if (left_count > 0) {
        *left_color = (uint16)(left_sum / left_count);
    } else {
        *left_color = 0;
    }
    
    if (right_count > 0) {
        *right_color = (uint16)(right_sum / right_count);
    } else {
        *right_color = 0;
    }
}

/*
 * 使用示例：
 * 
 * uint16 (*img_ptr)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
 * static uint16 mask[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH];
 * uint16 left_color, right_color, midline_x;
 * 
 * // 获取图像缓冲区
 * img_ptr = (uint16 (*)[CV_IMAGE_HEIGHT][CV_IMAGE_WIDTH])Camera_GetLatest();
 * 
 * if (img_ptr != NULL) {
 *     // 生成二值化掩码
 *     CV_PreprocessImage(img_ptr, &mask);
 *     
 *     // 获取动态中线左右两侧的颜色（在Y=100位置，左右各检测10个像素）
 *     CV_GetDynamicMidlineSideColors(img_ptr, &mask, 100, 10, 10, &left_color, &right_color, &midline_x);
 *     
 *     // 使用颜色值和中线位置进行判断
 *     printf("中线位置: %d, 左侧颜色: %d, 右侧颜色: %d\n", midline_x, left_color, right_color);
 *     
 *     if (left_color > right_color) {
 *         // 左侧颜色较亮
 *     } else if (right_color > left_color) {
 *         // 右侧颜色较亮
 *     } else {
 *         // 两侧颜色相近
 *     }
 *     
 *     // 释放图像缓冲区
 *     Camera_Release(img_ptr);
 * }
 */

