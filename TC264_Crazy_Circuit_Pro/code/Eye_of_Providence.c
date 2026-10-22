/*
 * Eye_of_Providence.c
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */
#include "zf_common_headfile.h"
static RoadType_e Protect_Last_Road_Type = Normol;
static uint8_t Route_Map_Last_Index = 0xFF;
static uint8_t Route_Map_Element_Armed = 1;
static uint8_t Route_Map_Normal_Frame_Count = 0;
static uint8_t Route_Map_Rearm_Frame_Count = 0;
#define ROUTE_MAP_REARM_NORMAL_FRAMES  (5)
#define ROUTE_MAP_REARM_MIN_FRAMES     (8)
#define ROUTE_MAP_REARM_MIN_DISTANCE   CM_TO_ENCODER(15)
#define ROUTE_MAP_REARM_MAX_YAW_RATE   (2500)
static const RoadType_e Route_Path0_Expected_Type[26] = {
    RightAngle_R, Node_R, Node_L, Node_L, Node_R,
    Node_R, Node_T, RightAngle_L, Node_L, Node_L,
    Node_R, RightAngle_L, Node_T, Node_R, Node_R,
    Node_T, Node_T, Node_L, Node_L, RightAngle_L,
    Node_L, Node_L, Node_T, Node_T, Node_R,
    RightAngle_R
};
static const int Route_Path0_Fallback_Distance[26] = {
    0, 0, 0, CM_TO_ENCODER(20), 0,
    0, 0, 0, 0, 0,
    0, CM_TO_ENCODER(25), CM_TO_ENCODER(40), 0, 0,
    0, CM_TO_ENCODER(60), 0, 0, 0,
    0, 0, 0, 0, 0,
    0
};
int route_map_enable = 1;
int route_approach_speed = 300;
volatile int route_expected_road_type = Normol;
volatile int route_raw_road_type = Normol;
volatile int route_fallback_active = 0;
volatile int route_fallback_count = 0;
volatile int route_fallback_distance = 0;
volatile int route_rearm_active = 0;
volatile int route_rearm_frames = 0;

#define RACE_IMAGE_WIDTH             (80)
#define RACE_IMAGE_HEIGHT            (60)
#define RACE_IMAGE_PACKED_BYTES      (600)
#define RACE_IMAGE_PRE_FRAMES        (2)
#define RACE_IMAGE_POST_FRAMES       (3)
#define RACE_IMAGE_CLIP_FRAMES       (6)
#define RACE_IMAGE_CLIP_COUNT        (3)

typedef struct
{
    uint32_t tick_5ms;
    uint32_t camera_sequence;
    int distance;
    int error;
    uint8_t node_index;
    uint8_t road_type;
    uint8_t expected_type;
    uint8_t raw_type;
    uint8_t fallback_active;
    uint8_t offline;
    uint8_t first_w_left;
    uint8_t first_w_right;
    uint8_t spinodal_left;
    uint8_t spinodal_right;
    uint8_t image[RACE_IMAGE_PACKED_BYTES];
} RaceImageFrame_t;

typedef struct
{
    uint8_t event_type;
    uint8_t capturing;
    volatile uint8_t ready;
    uint8_t sent;
    uint8_t frame_count;
    uint8_t post_remaining;
    RaceImageFrame_t frames[RACE_IMAGE_CLIP_FRAMES];
} RaceImageClip_t;

static RaceImageFrame_t Race_Image_Pre[RACE_IMAGE_PRE_FRAMES];
static RaceImageFrame_t Race_Image_Current;
static RaceImageClip_t Race_Image_Clips[RACE_IMAGE_CLIP_COUNT];
static uint8_t Race_Image_Pre_Write = 0;
static uint8_t Race_Image_Pre_Count = 0;
static uint8_t Race_Image_Armed = 0;
static uint8_t Race_Image_Tx_Clip = 0;
static uint8_t Race_Image_Tx_Frame = 0;
static uint32_t Race_Image_Camera_Sequence = 0;
static char Race_Image_Send_Buffer[1024];
ImageStatustypedef ImageStatus;                     // ͼ����״̬
ImageDealDatatypedef ImageDeal[60];                // ��¼ÿ�е������߽���Ϣ
ImageFlagtypedef ImageFlag;                        // ͼ��Ԫ��ʶ���־λ
RoadType_lock Element_lock;                        // Ԫ��������ʹ�ܿ��ƣ�
ImageFrameDetecttypedef ImageFrameDetect;          // ͼ��֡���ṹ��
Pathtypedef Path[3];                               // ·������
uint8_t mt9_copy_image[MT9V03X_1_H][MT9V03X_1_W - 8];          // ���ƺ��ԭʼͼ������
uint8_t mt9_compress_image[MT9_COMPRESS_H][MT9_COMPRESS_W];     // ѹ�����ͼ�����飨80x60��
uint8_t mt9_image_show[MT9_COMPRESS_H][MT9_COMPRESS_W];         // ���ڲ˵���ʾ��ͼ������
uint8_t mt9_binary_image[MT9_COMPRESS_H][MT9_COMPRESS_W];       // ��ֵ�����ͼ������
uint8_t mt9_binary_image_wifi_show[MT9_COMPRESS_H][MT9_COMPRESS_W]; // WiFi�����õĶ�ֵ��ͼ��
static uint8_t BottomBorderRight = 79,             // ��59�У����У��ұ߽�
             BottomBorderLeft = 0,                 // ��59�У����У���߽�
             BottomCenter = 0;                     // ��59�У����У����ĵ�
static uint8_t* PicTemp;                           // ����ͼ������ָ�룬���ڶ�ȡ��ֵ��ֵ���кڰ��жϣ�1=��, 0=�ڣ�
int ImageScanInterval_small;                       // С��Χɨ��������
static int IntervalLow = 0, IntervalHigh = 0;      // ���ұ߽��ɨ������
static int Ysite = 0, Xsite = 0;                  // Y=������, X=������
int threshold_total = 0;                           // ȫ�ִ����ֵ
int threshold_far = 0;                             // Զ�˶�ֵ����ֵ
int threshold_middle = 0;                          // �м��ֵ����ֵ
int threshold_near = 0;                            // ���˶�ֵ����ֵ
uint8_t OSTU_Low = 0;                              // �����ֵ����
uint8_t OSTU_High = 0;                             // �����ֵ����
uint8_t whitehigh = 0;                             // ���߸߶ȣ��ӵ������ϵ���Ч������������
float whitehigh_float = 0;                         // ���ڼ��㶯̬ǰհ�İ��߸߶ȹ�һ��ֵ
int Gain_High = 9;                                 // ǰհ��������
int Gain_Low = -3;                                 // ǰհ��������
int TowPoint = 0;                                  // ��ǰǰհ���к�
float DetTemp = 0;                                 // ƫ�������ʱֵ
int Ysite_bottom = 0;                              // ƫ�������ʼ��
int Ysite_top = 0;                                 // ƫ����������
float gamma_value = 0.5;                           // GammaУ��������<1��ǿ�Աȶ�, >1���ͶԱȶȣ�
// ������������������飨����Ԫ��ʶ��ʱ�Ŀ�ȶԱȣ��±�Ϊ�к�
uint8_t Image_Real_Wide[60] = {
    3, 3, 4, 5, 5, 5, 5, 5, 5, 5,   // ��0-9��
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,   // ��10-19��
    5, 5, 6, 6, 6, 6, 6, 6, 6, 6,   // ��20-29��
    6, 6, 6, 6, 6, 6, 6, 6, 7, 7,   // ��30-39��
    7, 7, 7, 7, 7, 7, 8, 8, 8, 8,   // ��40-49��
    8, 8, 8, 8, 8, 8, 8, 8, 8, 9    // ��50-59��
};
// 20����̬�ֲ�Ȩ�أ����ĶԳƣ����ڼ�Ȩ����ƫ�
float Weighting_20[20] = {
    0.020, 0.032, 0.050, 0.076, 0.112,  // 0-4
    0.158, 0.215, 0.284, 0.364, 0.450,  // 5-9
    0.450, 0.364, 0.284, 0.215, 0.158,  // 10-14
    0.112, 0.076, 0.050, 0.032, 0.020   // 15-19
};
int Err = 0;
uint8_t lut[256];
void Race_Image_Reset(void)
{
    Race_Image_Armed = 0;
    memset(Race_Image_Pre, 0, sizeof(Race_Image_Pre));
    memset(&Race_Image_Current, 0, sizeof(Race_Image_Current));
    memset(Race_Image_Clips, 0, sizeof(Race_Image_Clips));
    Race_Image_Pre_Write = 0;
    Race_Image_Pre_Count = 0;
    Race_Image_Tx_Clip = 0;
    Race_Image_Tx_Frame = 0;
    Race_Image_Camera_Sequence = 0;
}

static void Race_Image_Pack_Current(void)
{
    int x;
    int y;
    int byte_index;
    uint8_t bit_mask;

    memset(&Race_Image_Current, 0, sizeof(Race_Image_Current));
    Race_Image_Current.tick_5ms = race_tick_5ms;
    Race_Image_Current.camera_sequence = Race_Image_Camera_Sequence++;
    Race_Image_Current.distance = encoder.disA;
    Race_Image_Current.error = Err;
    Race_Image_Current.node_index = Path[Path_Switch].target_angle_index;
    Race_Image_Current.road_type = (uint8_t)ImageStatus.Road_type;
    Race_Image_Current.expected_type = (uint8_t)route_expected_road_type;
    Race_Image_Current.raw_type = (uint8_t)route_raw_road_type;
    Race_Image_Current.fallback_active = (uint8_t)route_fallback_active;
    Race_Image_Current.offline = ImageStatus.OFFLine;
    Race_Image_Current.first_w_left = (uint8_t)ImageStatus.First_W_Point_L;
    Race_Image_Current.first_w_right = (uint8_t)ImageStatus.First_W_Point_R;
    Race_Image_Current.spinodal_left = (uint8_t)ImageFrameDetect.spinodal_L_Ysite;
    Race_Image_Current.spinodal_right = (uint8_t)ImageFrameDetect.spinodal_R_Ysite;

    for(y = 0; y < RACE_IMAGE_HEIGHT; y++)
    {
        for(x = 0; x < RACE_IMAGE_WIDTH; x++)
        {
            if(mt9_binary_image[y][x])
            {
                byte_index = y * (RACE_IMAGE_WIDTH / 8) + x / 8;
                bit_mask = (uint8_t)(1U << (7 - (x & 7)));
                Race_Image_Current.image[byte_index] |= bit_mask;
            }
        }
    }
}

static void Race_Image_Advance_Clips(void)
{
    int clip_index;
    RaceImageClip_t *clip;

    for(clip_index = 0; clip_index < RACE_IMAGE_CLIP_COUNT; clip_index++)
    {
        clip = &Race_Image_Clips[clip_index];
        if(!clip->capturing || clip->post_remaining == 0) continue;

        if(clip->frame_count < RACE_IMAGE_CLIP_FRAMES)
        {
            memcpy(&clip->frames[clip->frame_count], &Race_Image_Current, sizeof(RaceImageFrame_t));
            clip->frame_count++;
        }

        clip->post_remaining--;
        if(clip->post_remaining == 0)
        {
            clip->capturing = 0;
            clip->ready = 1;
        }
    }
}

static void Race_Image_Start_Clip(int clip_index, int event_type)
{
    int i;
    int source_index;
    RaceImageClip_t *clip;

    if(clip_index < 0 || clip_index >= RACE_IMAGE_CLIP_COUNT) return;
    clip = &Race_Image_Clips[clip_index];
    if(clip->capturing || clip->ready || clip->frame_count > 0) return;

    clip->event_type = (uint8_t)event_type;
    for(i = 0; i < Race_Image_Pre_Count; i++)
    {
        source_index = (Race_Image_Pre_Write + RACE_IMAGE_PRE_FRAMES
            - Race_Image_Pre_Count + i) % RACE_IMAGE_PRE_FRAMES;
        memcpy(&clip->frames[clip->frame_count], &Race_Image_Pre[source_index], sizeof(RaceImageFrame_t));
        clip->frame_count++;
    }

    memcpy(&clip->frames[clip->frame_count], &Race_Image_Current, sizeof(RaceImageFrame_t));
    clip->frame_count++;
    clip->post_remaining = RACE_IMAGE_POST_FRAMES;
    clip->capturing = 1;
}

static void Race_Image_Update_Prebuffer(void)
{
    memcpy(&Race_Image_Pre[Race_Image_Pre_Write], &Race_Image_Current, sizeof(RaceImageFrame_t));
    Race_Image_Pre_Write = (uint8_t)((Race_Image_Pre_Write + 1) % RACE_IMAGE_PRE_FRAMES);
    if(Race_Image_Pre_Count < RACE_IMAGE_PRE_FRAMES) Race_Image_Pre_Count++;
}

void Race_Image_Frame_Update(void)
{
    if(race_state == RACE_STATE_MOTOR_RAMP || race_state == RACE_STATE_RUNNING)
    {
        Race_Image_Armed = 1;
    }

    if(!Race_Image_Armed) return;

    Race_Image_Pack_Current();
    Race_Image_Advance_Clips();

    if(route_raw_road_type != Normol
        && route_expected_road_type != Normol
        && route_raw_road_type != route_expected_road_type)
    {
        Race_Image_Start_Clip(0, 1);
    }

    if(route_fallback_active)
    {
        Race_Image_Start_Clip(1, 2);
    }

    if(race_stop_reason != RACE_STOP_NONE
        || race_state == RACE_STATE_STOPPING
        || race_state == RACE_STATE_STOPPED)
    {
        Race_Image_Start_Clip(2, 3);
    }

    Race_Image_Update_Prebuffer();

    if(Race_Image_Clips[2].ready)
    {
        Race_Image_Armed = 0;
    }
}

static int Race_Image_Base64_Encode(const uint8_t *input, int length, char *output)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int input_index = 0;
    int output_index = 0;
    uint32_t value;
    int remaining;

    while(input_index < length)
    {
        remaining = length - input_index;
        value = ((uint32_t)input[input_index]) << 16;
        if(remaining > 1) value |= ((uint32_t)input[input_index + 1]) << 8;
        if(remaining > 2) value |= input[input_index + 2];

        output[output_index++] = table[(value >> 18) & 0x3F];
        output[output_index++] = table[(value >> 12) & 0x3F];
        output[output_index++] = remaining > 1 ? table[(value >> 6) & 0x3F] : '=';
        output[output_index++] = remaining > 2 ? table[value & 0x3F] : '=';
        input_index += 3;
    }

    return output_index;
}

void Race_Image_Transmit_Update(void)
{
    RaceImageClip_t *clip;
    RaceImageFrame_t *frame;
    int length;
    int encoded_length;

    if(Wireless_init_flag || race_state != RACE_STATE_STOPPED) return;

    while(Race_Image_Tx_Clip < RACE_IMAGE_CLIP_COUNT)
    {
        clip = &Race_Image_Clips[Race_Image_Tx_Clip];
        if(clip->ready && !clip->sent) break;
        Race_Image_Tx_Clip++;
        Race_Image_Tx_Frame = 0;
    }

    if(Race_Image_Tx_Clip >= RACE_IMAGE_CLIP_COUNT) return;
    clip = &Race_Image_Clips[Race_Image_Tx_Clip];

    if(Race_Image_Tx_Frame >= clip->frame_count)
    {
        clip->sent = 1;
        Race_Image_Tx_Clip++;
        Race_Image_Tx_Frame = 0;
        return;
    }

    frame = &clip->frames[Race_Image_Tx_Frame];
    length = sprintf(Race_Image_Send_Buffer,
        "IMG,1,%d,%d,%d,%d,%lu,%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
        clip->event_type,
        Race_Image_Tx_Clip,
        Race_Image_Tx_Frame,
        clip->frame_count,
        (unsigned long)frame->tick_5ms,
        (unsigned long)frame->camera_sequence,
        frame->node_index,
        frame->distance,
        frame->road_type,
        frame->expected_type,
        frame->raw_type,
        frame->fallback_active,
        frame->offline,
        frame->error,
        frame->first_w_left,
        frame->first_w_right,
        frame->spinodal_left,
        frame->spinodal_right);

    encoded_length = Race_Image_Base64_Encode(
        frame->image,
        RACE_IMAGE_PACKED_BYTES,
        &Race_Image_Send_Buffer[length]);
    length += encoded_length;
    Race_Image_Send_Buffer[length++] = '\r';
    Race_Image_Send_Buffer[length++] = '\n';
    Race_Image_Send_Buffer[length] = '\0';

    if(wifi_spi_send_buffer((const uint8 *)Race_Image_Send_Buffer, (uint32)length) == 0)
    {
        Race_Image_Tx_Frame++;
    }
}
int continue_lost_count = 0;
uint8_t Image_Solve_Finish_Flag = 0;  // ͼ������ɱ�־
// ����һ֡ͼ���������8����Ч���أ��ӵ�8�п�ʼȡ���ݣ�
void copy_img(void)  // ����ͼ��->ֱ��ȥ�����14����Ч����
{
    for (int i = 0; i < MT9V03X_1_H; i++)
    {
        memcpy(&mt9_copy_image[i][0],&mt9v03x_image_1[i][8],MT9V03X_1_W-8);
    }
    mt9v03x_finish_flag_1 = 0;  // ʹ����һ֡DMA����ͼ�񣬿��Կ�ʼ������һ֡
}
// ͼ��ѹ������
float Myh = MT9V03X_1_H;  // ԭʼͼ��߶�
float Compress_h = MT9_COMPRESS_H;  // ѹ����ͼ��߶�
float Myw = MT9V03X_1_W - 8 ;  // ԭʼͼ����
float Compress_w = MT9_COMPRESS_W;  // ѹ����ͼ����
/*
 * �������ƣ�void img_compress(uint8 img[MT9V03X_H][MT9V03X_W])
 * ����˵����ͼ��ѹ��
 * ����˵������
 * ����ֵ����
 * ʾ����img_compress(img); // ���ձ�����ԭʼͼ����в����ﵽѹ��ͼ���Ŀ��
 */
void img_compress(void)  // ͼ��ѹ��->��160x120��ͼ��ѹ��Ϊ80x60
{
    int j, k, row, line;
    const float div_h = Myh / Compress_h, div_w = Myw / Compress_w;
    for (j = 0; j < MT9_COMPRESS_H; j++)
    {
        row = j * div_h + 0.5;  // �����Ӧ��
        for (k = 0; k < MT9_COMPRESS_W; k++)
        {
            line = k * div_w + 0.5;
            mt9_compress_image[j][k] = mt9_copy_image[row][line];  // ʹ��ͬһ������
        }
    }
}
// ͼ��ѹ��2����
float Myh2 = MT9V03X_1_H;  // ԭʼͼ��߶�
float Compress_h2 = MT9_COMPRESS_H;  // ѹ����ͼ��߶�
float Myw2 = MT9V03X_1_W;  // ԭʼͼ����
float Compress_w2 = MT9_COMPRESS_W;  // ѹ����ͼ����
void img_compress2(void)  // ͼ��ѹ��2�����ڲ˵���ʾ��ԭͼ��
{
    int j, k, row, line;
    const float div_h = Myh2 / Compress_h2, div_w = Myw2 / Compress_w2;
    for (j = 0; j < MT9_COMPRESS_H; j++)
    {
        row = j * div_h + 0.5;
        for (k = 0; k < MT9_COMPRESS_W; k++)
        {
            line = k * div_w + 0.5;
            mt9_image_show[j][k] = mt9v03x_image_1[row][line];  // ���ڲ˵���ʾ��ԭͼ
        }
    }
}
// GammaУ��
void gamma_init(float gamma) {  // gamma<1.0���Աȶ���ǿ; gamma>1.0���Աȶȼ���
    for (int i = 0; i < 256; i++) {
        lut[i] = (unsigned char)(pow(i / 255.0, 1.0 / gamma) * 255);
    }
}
void gamma_correction(unsigned char* img, int size) {
    for (int i = 0; i < size; i++) {
        img[i] = lut[img[i]];
    }
}
int Dajin_of_part(uint8_t *img_data, uint32_t pixel_count)  // ͼ��������ֵ������ͼ���ΪԶ�ˡ��м䡢���������֣��ֱ���㲻ͬ�Ķ�ֵ����ֵ��ʵ�ָ��õĶ�ֵ��Ч����ÿ����Ϊ20�У�
{
    //  ��򷨣�Otsu method��ʵ��
    //  1. ����Ҷ�ֱ��ͼ
    uint32_t histogram[256] = {0};
    uint32_t total_pixels = pixel_count;

    //  ͳ��ÿ���Ҷȼ����ֵ�������
    for (uint32_t i = 0; i < pixel_count; i++)
    {
        histogram[img_data[i]]++;
    }

    //  2. �����ܻҶ�ֵ
    uint32_t sum_total = 0;
    for (int i = 0; i < 256; i++)
    {
        sum_total += i * histogram[i];
    }
    
    //  3. ��򷨼��������ֵ
    uint32_t sum_background = 0;  //      �����Ҷ��ܺ�
    uint32_t weight_background = 0;  //      ������������
    uint32_t weight_foreground;  //      ǰ����������

    float variance_max = 0;  //      �����䷽��
    int threshold = 0;  //      �����ֵ
    
    for (int t = 0; t < 256; t++)
    {
        weight_background += histogram[t];
        if (weight_background == 0) continue;
        
        weight_foreground = total_pixels - weight_background;
        if (weight_foreground == 0) break;
        
        sum_background += t * histogram[t];

        //      ���㱳����ǰ���ľ�ֵ
        float mean_background = (float)sum_background / weight_background;
        float mean_foreground = (float)(sum_total - sum_background) / weight_foreground;

        //      ������䷽��
        float variance_between = (float)weight_background * (float)weight_foreground *
                               (mean_background - mean_foreground) * (mean_background - mean_foreground);

        //      �ҳ���󷽲��Ӧ��ֵ
        if (variance_between > variance_max)
        {
            variance_max = variance_between;
            threshold = t;
        }
    }
    
    return threshold;
}
/*************************************************************************
 *  �������ƣ�uint16 GetOSTU(uint8 OSTUImage[MT9V03X_H][MT9V03X_W])
 *  ����˵����������ֵ��С
 *  ����˵����OSTUImage -- ͼ������
 *  ����ֵ��uint8_t ��ֵ
 *  ��ע��GetOSTU(Image_Use); // ȡ��ֵ
Ostu���: �����䷽���ͨ��ͳ������ͼ���ֱ��ͼ��ʵ��ȫ����ֵT���Զ�ѡȡ�����㷨����Ϊ��
1) �ȼ���ͼ���ֱ��ͼ����ͼ�����е����ص㰴0~255��256��bin��ͳ������ÿ��bin����������
2) ��һ��ֱ��ͼ��Ҳ��ÿ��bin���������������ܵ�������
3) i��ʾ�������ֵ��Ҳ��һ���Ҷȼ�����0��ʼ����
4) ͨ����һ����ֱ��ͼ��ͳ��0~i�Ҷȼ�������(ǰ������)��ռ����ͼ��ı���w0����ͳ��ǰ�����ص�ƽ���Ҷ�u0��
       ͳ��i~255�Ҷȼ�������(��������)��ռ����ͼ��ı���w1����ͳ�Ʊ������ص�ƽ���Ҷ�u1��
5) ����ǰ�����غͱ������صķ��� g = w0*w1*(u0-u1)*(u0-u1)
6) i++��ת��4)��ֱ��iΪ256ʱ��������
7) ������g��Ӧ��iֵ��Ϊͼ���ȫ����ֵ
ȱ��: OSTU�㷨�ڴ�����ղ����ȵ�ͼ��ʱ��Ч�����Բ��ã���Ϊ���õ���ȫ��ͼ����Ϣ��
*************************************************************************/
uint8_t GetOSTU(uint8_t *OSTUImage,uint16_t width,uint16_t height)
{
    int X;uint16_t Y;
    uint32_t Amount = 0;
    uint32_t PixelBack = 0;  // ǰ����������
    uint32_t PixelshortegralBack = 0;  // ǰ���Ҷ�ֵ
    uint32_t  Pixelshortegral = 0;  // �����Ҷ�ֵ
    int32_t  PixelshortegralFore = 0;  // �����Ҷ�
    int32_t  PixelFore = 0;  // ������������
    double  OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;  //  ��䷽��;
    uint8_t  MinValue, MaxValue;
    uint8_t  Threshold = 0;
    int HistoGram[256] = {0};  // ��ʼ���Ҷ�ֱ��ͼ
    for (Y = 0; Y < height; Y++)
    {
        for (X = 0; X < width; X++)
        {
            HistoGram[OSTUImage[Y*width + X]]++;  // ͳ�ƻҶȼ�ÿ��������ͼ���еĸ���
        }
    }
    for (MinValue = 0; MinValue < 256 && HistoGram[MinValue] == 0; MinValue++);  // ��ȡ��С�Ҷȵ�ֵ
    for (MaxValue = 255; MaxValue > MinValue && HistoGram[MaxValue] == 0; MaxValue--);  // ��ȡ���Ҷȵ�ֵ
    if (MaxValue == MinValue)
        return MaxValue;  //  ͼ����ֻ��һ����ɫ
    if (MinValue + 1 == MaxValue)
        return MinValue;  //  ͼ����ֻ�ж�����ɫ
    for (Y = MinValue; Y <= MaxValue; Y++)
        Amount += HistoGram[Y];  //   ��������
    Pixelshortegral = 0;
    for (Y = MinValue; Y <= MaxValue; Y++)
    {
        Pixelshortegral += HistoGram[Y] * Y;  // �Ҷ�ֵ�ܺ�
    }
    SigmaB = -1;
    for (Y = MinValue; Y < MaxValue; Y++)
    {
        PixelBack = PixelBack + HistoGram[Y];  // ǰ����������
        PixelFore = Amount - PixelBack;  // ������������
        OmegaBack = (float) PixelBack / Amount;  // ǰ�����ذٷֱ�
        OmegaFore = (float) PixelFore / Amount;  // �������ذٷֱ�
        PixelshortegralBack += HistoGram[Y] * Y;  // ǰ���Ҷ�ֵ
        PixelshortegralFore = Pixelshortegral - PixelshortegralBack;  // �����Ҷ�ֵ
        MicroBack = (float) PixelshortegralBack / PixelBack;  // ǰ���ҶȰٷֱ�
        MicroFore = (float) PixelshortegralFore / PixelFore;  // �����ҶȰٷֱ�
        Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);  // ������䷽��
        if (Sigma > SigmaB)  // �ҳ������䷽��g�Լ���Ӧ����ֵ
        {
            SigmaB = Sigma;
            Threshold = (uint8_t)Y;
        }
    }
    return Threshold;  // ���������ֵ;
}
void binary_by_part(void)  // ͼ��������ֵ������ͼ���ΪԶ�ˡ��м䡢���������֣��ֱ����ò�ͬ�Ķ�ֵ����ֵ��ʵ�ָ��õĶ�ֵ��Ч����ÿ����Ϊ20�У�
{
    // ȫ�ִ��
    threshold_total = GetOSTU((uint8_t *)mt9_compress_image, MT9_COMPRESS_W, MT9_COMPRESS_H);
    if(threshold_total < OSTU_Low) threshold_total = OSTU_Low;
    else if(threshold_total > OSTU_High) threshold_total = OSTU_High;
    threshold_far = threshold_total -2;
    threshold_middle = threshold_total -1 ;
    threshold_near = threshold_total +1;
    // ͼ���ֵ��
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < MT9_COMPRESS_W; j++)
        {
            if (mt9_compress_image[i][j] >= threshold_far)   mt9_binary_image[i][j] = 1;
            else   mt9_binary_image[i][j] = 0;
            if (mt9_compress_image[i+20][j] >= threshold_middle)   mt9_binary_image[i+20][j] = 1;
            else   mt9_binary_image[i+20][j] = 0;
            if (mt9_compress_image[i+40][j] >= threshold_near)   mt9_binary_image[i+40][j] = 1;
            else   mt9_binary_image[i+40][j] = 0;
        }
    }
}
void Fine_whitepoint(void)  // Ѱ���������
{
    uint8_t black_high = 0;  // ���߸߶�
    for(black_high = 2; black_high < 60; black_high++)
    {
        if((ImageDeal[black_high].IsLeftFind == 'T' || ImageDeal[black_high].IsLeftFind == 'W') 
        && (ImageDeal[black_high].IsRightFind == 'T' || ImageDeal[black_high].IsRightFind == 'W'))  // ����ҵ����ұ߽綼Ϊ�ױ�
        {
            ImageStatus.OFFLine = black_high;    
            break;
        }
    }
    if(ImageStatus.OFFLine > 58) ImageStatus.OFFLine = 58;
    if(ImageStatus.OFFLine < 2) ImageStatus.OFFLine = 2;
    
    whitehigh = 59 - ImageStatus.OFFLine;  // ������߿��
    whitehigh_float = (float)(whitehigh - 40)/(60 - 40) + 0.5;  // ���ڼ��㶯̬ǰհ
}
// ��һ����������ҪѰ�ҵ����飨80���㣩
// �ڶ���������ɨ����߻���ɨ���ұ�
// �������ǿ�ʼ�ͽ�����
// ���ĸ������Ƿ��ص�����㣨��Ҫ����ָ�룩
void GetJumpPointFromDet(uint8_t* p,uint8_t type,int L,int H,JumpPointtypedef* JumpPoint)
{
    int i = 0;
    if (type == 'L')  // ɨ�������
    {
        for (i = L; i <= H; i++)  // ��������ɨ��
        {
            if (*(p + i) == 0 && *(p + i + 1) == 1)  // �ɺڱ�׼�Ϊ�ҵ�
            {
                JumpPoint->point = i;  // ��¼
                JumpPoint->type = 'T';
                break;
            }
            else if (i == (H - 1))  // ���ɨ�赽����Ҳû�ҵ�
            {
                if (*(p + (L + H) / 2) == 1)  // ����м��ǰ׵�
                {
                    JumpPoint->point = (L + H) / 2;  // �ұ�ȡ�м��
                    JumpPoint->type = 'W';  // �޷�ȷ�����м�Ϊ�ף���Ϊû�бߣ����ǰ׵�->��Ϊ�Ǵ�ֱ����ͨ�������жϴ���
                    break;
                }
                else  // ����м��Ǻڵ�
                {
                    JumpPoint->point = (L + H) / 2;  // �ұ�ȡ��Сֵ
                    JumpPoint->type = 'H';  // �޷�ȷ�����м�Ϊ�ڣ���Ϊû�бߣ����Ǻڵ�->��Ϊ���ϰ��￿�ڱ�����
                    break;
                }
            }
        }
    }
    else if (type == 'R')  // ɨ���ұ���
    {
        for (i = H; i >= L; i--)
        {
            if (*(p + i) == 0 && *(p + i - 1) == 1)  // �ɺڱ��
            {
                JumpPoint->point = i;  // ��¼�����
                JumpPoint->type = 'T';  // ��ȷ��
                break;
            }
            else if (i == (L + 1))  // ���ɨ�赽����Ҳû�ҵ�
            {
                if (*(p + (L + H) / 2) == 1)  // ����м��ǰ׵�
                {
                    JumpPoint->point = (L + H) / 2;  // ��Ϊ�м����е�
                    JumpPoint->type = 'W';  // �޷�ȷ�����м�Ϊ�ף���Ϊû�бߣ����ǰ׵�
                    break;
                }
                else  // �޷�ȷ�����м�Ϊ��
                {
                    JumpPoint->point = (L + H) / 2;  // ����м��Ǻڵ�
                    JumpPoint->type = 'H';  // ȡ��Сֵ��Ϊ��ֱ��
                    break;
                }
            }
        }
    }
}
// ��ȡ��ʵ�����ұ߽磬��Ҫ����Ԫ�صĸ����ж�
void Get_Actually_L_R_Line(int row, uint8_t type,JumpPointtypedef* JumpPoint)
{
    if (type == 'L')
    {
        ImageDeal[row].LeftBorder2 = 1;  // Ĭ����߽�Ϊ�����
        if (JumpPoint->type == 'H' || JumpPoint->type == 'W')  // Ϊ�ϰ�����ޱ߽߱�
        {
            for (int i = MT9_COMPRESS_W / 2; i > 0; i--)
            {
                if (mt9_binary_image[row][i + 1] == 1 && mt9_binary_image[row][i] == 0)  // ���м�����Ѱ�Ұ׺�����
                {
                    ImageDeal[row].LeftBorder2 = i;
                    break;
                }
            }
        }
        else if(JumpPoint->type == 'T')  // �������
            ImageDeal[row].LeftBorder2 = ImageDeal[row].LeftBorder;
    }
    else if (type == 'R')
    {
        ImageDeal[row].RightBorder2 = 79;
        if (JumpPoint->type == 'H' || JumpPoint->type == 'W')  // Ϊ�ϰ�����ޱ߽߱�
        {
            for (int i = MT9_COMPRESS_W / 2; i < MT9_COMPRESS_W - 1; i++)
            {
                if (mt9_binary_image[row][i] == 1 && mt9_binary_image[row][i + 1] == 0)  // ���м�����Ѱ�Ұ׺�����
                {
                    ImageDeal[row].RightBorder2 = i;
                    break;
                }
            }
        }
        else if(JumpPoint->type == 'T')  // �������
            ImageDeal[row].RightBorder2 = ImageDeal[row].RightBorder;
    }
}
// ���Ƶױߣ���Ҫ��ȷ�ҵ������ױ߲��Դ�ѭ��
// �Եױ�Ҫ���ϸ�
static void DrawLinesFirst(void)
{
    PicTemp = mt9_binary_image[59];  // ������п�ʼ
    // ����м��ǰ׵������£���������һֱ�ң����ҵ��򷵻ر߽磬û���ҵ����ǰ׵ף�����ʱ��߽���ұ߽綼Ϊ39
    for (Xsite = 79; Xsite > 0; Xsite--)  // һ��һ�еش���������
    {  // �Ӻڵ��ף����������׵㼴��Ϊ�Ǻڰ�����
        if (*(PicTemp + Xsite) == 1 &&
            *(PicTemp + Xsite - 1) == 1)  // ������������׵㣬�˲�
        {
            BottomBorderRight = Xsite;  // �ҵ�����¼
            break;
        }
        else if (Xsite == 1)
        {
            BottomBorderRight = 39;  // �Ҳ�����Ϊ39
            break;
        }
    }
    for (Xsite = 0; Xsite < 79; Xsite++)  // һ��һ�еش���������
    {
        if (*(PicTemp + Xsite) == 1 &&
            *(PicTemp + Xsite + 1) == 1)  // ������������׵㣬�˲�
        {
            BottomBorderLeft = Xsite;  // �ҵ�����¼
            break;
        }
        else if (Xsite == 78)
        {
            BottomBorderLeft = 39;  // �Ҳ�����Ϊ39
            break;
        }
    }
    BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;  //  59���е�ֱ��ȡƽ��
    ImageDeal[59].LeftBorder = BottomBorderLeft;  // ��¼������Ϣ���飬����һ�вο�
    ImageDeal[59].RightBorder = BottomBorderRight;
    ImageDeal[59].Center = BottomCenter;  // ȷ���ױ�
    ImageDeal[59].Wide = BottomBorderRight - BottomBorderLeft;  // �洢�����Ϣ
    ImageDeal[59].IsLeftFind = 'T';
    ImageDeal[59].IsRightFind = 'T';
}
int long_white_flag_L = 0;
int long_white_flag_R = 0;
// �������ϼ��������߽�
static void DrawLinesProcess(void)  // ���ĺ���
{
    // ̫Զ��ͼ�񲻴����OFFLine�Ժ�Ĳ�����
    for (Ysite = 58 ; Ysite >= ImageStatus.OFFLine; Ysite--)  // ǰ5�д�����ˣ����Դ�58�е�Ԥ���趨�Ķ�ʧ��OFFLine
    {                        
        PicTemp = mt9_binary_image[Ysite];
        JumpPointtypedef JumpPoint[2];  //  0��1��->���ұ�
        IntervalLow =ImageDeal[Ysite + 1].LeftBorder - ImageScanInterval_small;  // ����һ�����-5�ĵ㿪ʼ��ȷ��ɨ�迪ʼ�㣩
        IntervalHigh =ImageDeal[Ysite + 1].LeftBorder + ImageScanInterval_small;  // ����һ�����+5�ĵ������ȷ��ɨ������㣩
        LimitL(IntervalLow);  // ȷ����ɨ�����䲢���Ʒ�Χ
        LimitH(IntervalHigh);  // ȷ����ɨ�����䲢���Ʒ�Χ
        GetJumpPointFromDet(PicTemp, 'L', IntervalLow, IntervalHigh,&JumpPoint[0]);
        IntervalLow =ImageDeal[Ysite + 1].RightBorder -ImageScanInterval_small;  // ����һ���ұ�-Interval�ĵ㿪ʼ��ȷ��ɨ�迪ʼ�㣩
        IntervalHigh =ImageDeal[Ysite + 1].RightBorder + ImageScanInterval_small;  // ����һ���ұ�+Interval�ĵ������ȷ��ɨ������㣩
        LimitL(IntervalLow);  // ȷ����ɨ�����䲢���Ʒ�Χ
        LimitH(IntervalHigh);  // ȷ����ɨ�����䲢���Ʒ�Χ
        GetJumpPointFromDet(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);  // ɨ���ұ���
        // ����ɷֿ��Ը����ж�ֱ�ǣ��ؼ����ж��Ƿ���ֱ��·�ڻ���T��·�ڣ���ҪС��Χ����ģ���Ļ���ֱ��ȡ��һ�еı߽�����
        ImageDeal[Ysite].LeftBorder = JumpPoint[0].point;  // ��¼�����
        ImageDeal[Ysite].RightBorder = JumpPoint[1].point;  // ��¼�ұ���
        ImageDeal[Ysite].IsLeftFind =JumpPoint[0].type;  // ��¼���ұ��Ƿ��ҵ��ߣ����ұ߽����ͣ�
        ImageDeal[Ysite].IsRightFind = JumpPoint[1].type;
        // ��Ҫȷ����Щ��ȷ���ı�Ե->����һ������ɨ�跶Χ���м��Ǻڵģ��б߽�
        if ( ImageDeal[Ysite].IsLeftFind == 'H' || ImageDeal[Ysite].IsRightFind == 'H' || ImageDeal[Ysite].IsLeftFind == 'W' || ImageDeal[Ysite].IsRightFind == 'W')
        {
            // ��������в�ȷ��
            if (ImageDeal[Ysite].IsLeftFind == 'H' || ImageDeal[Ysite].IsLeftFind == 'W')
            {  // ���ȫͼɨ��
                IntervalLow = 5;
                IntervalHigh = MT9_COMPRESS_W - 5;
                for(Xsite = IntervalLow; Xsite <= IntervalHigh; Xsite++)  // ������֮������ɨ��
                {
                    if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite + 1) == 1))  // �Ӻڱ��
                    {
                        ImageDeal[Ysite].LeftBorder =Xsite;  // ������ߵ��ұ��кڰ����䣬����Ϊ�µı���ֱ��ȡ��
                        ImageDeal[Ysite].IsLeftFind = 'T';
                        break;
                    }
                    else if(*(PicTemp + Xsite) == 1)  // ���һֱ�����ҵĶ��ǰ׵�
                    {
                        ImageDeal[Ysite].IsLeftFind = 'W';
                        ImageDeal[Ysite].LeftBorder = 5;
                        break;  // һֱ���ұ߶��ǰ׵����Ϊ�ǰױ�W��
                    }
                    else if (Xsite == IntervalHigh )
                    {
                        ImageDeal[Ysite].IsLeftFind = 'H';
                        ImageDeal[Ysite].LeftBorder =ImageDeal[Ysite+1].LeftBorder;  // ȡ��һ�еı���
                        break;
                    }
                }
            }
            if (ImageDeal[Ysite].IsRightFind == 'H' || ImageDeal[Ysite].IsRightFind == 'W')
            {  // �ұ�ȫͼɨ��
                IntervalLow = 5;
                IntervalHigh = MT9_COMPRESS_W - 5;
                for (Xsite = IntervalHigh; Xsite >= IntervalLow; Xsite--)
                {
                    if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite - 1) == 1))
                    {
                        ImageDeal[Ysite].RightBorder = Xsite;  // ���ұ��ߵ�����кڰ����䣬����Ϊ�µı���ֱ��ȡ��
                        ImageDeal[Ysite].IsRightFind = 'T';
                        break;
                    }
                    else if(*(PicTemp + Xsite) == 1)
                    {
                        ImageDeal[Ysite].IsRightFind = 'W';
                        ImageDeal[Ysite].RightBorder = 75;
                        break;  // һֱ����߶��ǰ׵����Ϊ�ǰױ�W��
                    }                        
                    else if (Xsite == IntervalLow)
                    {
                        ImageDeal[Ysite].IsRightFind = 'H';
                        ImageDeal[Ysite].RightBorder = ImageDeal[Ysite+1].RightBorder ;
                        break;
                    }
                }
            }
            if(ImageDeal[Ysite].IsLeftFind == 'H' && ImageDeal[Ysite].IsRightFind == 'H')   continue_lost_count++;
            else    continue_lost_count = 0;   
        }
        long_white_flag_L = 0;
        long_white_flag_R = 0;
        if(mt9_binary_image[Ysite][0] == 1 && mt9_binary_image[Ysite][5] == 1 && mt9_binary_image[Ysite][10] == 1) long_white_flag_L = 1;//����߽���ǰ׵㣬��Ϊ�ǰױ�
        if(mt9_binary_image[Ysite][70] == 1 && mt9_binary_image[Ysite][75] == 1 && mt9_binary_image[Ysite][79] == 1) long_white_flag_R = 1;//����߽���ǰ׵㣬��Ϊ�ǰױ�
        if(long_white_flag_L == 1 )//���֮ǰ�ж��Ǵ�����㵫�Ǳ߽���ǰ׵㣬��Ϊ�ǰױ�
        {
            ImageDeal[Ysite].IsLeftFind = 'W';
        }
        if(long_white_flag_R == 1 )//���֮ǰ�ж��Ǵ�����㵫�Ǳ߽���ǰ׵㣬��Ϊ�ǰױ�
        {
            ImageDeal[Ysite].IsRightFind = 'W';
        }       
        LimitL(ImageDeal[Ysite].LeftBorder);  // �޷�
        LimitH(ImageDeal[Ysite].LeftBorder);  // �޷�
        LimitL(ImageDeal[Ysite].RightBorder);  // �޷�
        LimitH(ImageDeal[Ysite].RightBorder);  // �޷�
        ImageDeal[Ysite].Wide = ImageDeal[Ysite].RightBorder - ImageDeal[Ysite].LeftBorder;  // ��ȡͼ����
        ImageDeal[Ysite].Center =(ImageDeal[Ysite].RightBorder + ImageDeal[Ysite].LeftBorder) / 2;
        // ��ǰ��ֹѲ��
        // ��ͼ����С��0�����ұߴﵽһ������ʱ����ǰ��ֹѲ��
        if(continue_lost_count > 5 && Ysite <= 10)
        {
            ImageStatus.OFFLine = Ysite + 5;
            break;
        }
        else if(continue_lost_count >= 25 && Ysite <= 35)  
        {
            ImageStatus.OFFLine = Ysite + 25;
            break;
        }
    }
}
void Frame_Detect(int center_x, int center_y, int frame_width, int frame_height)
{
    // ע�⣺����߽��������
    int Y_bottom = center_y + frame_height/2;
    int Y_top = center_y - frame_height/2;
    int X_right = center_x + frame_width/2;
    int X_left = center_x - frame_width/2;
    ImageFrameDetect.frame_bottom_flag = 0;
    ImageFrameDetect.frame_top_flag = 0;
    ImageFrameDetect.frame_right_flag = 0;
    ImageFrameDetect.frame_left_flag = 0;
    // �޷�
    Y_bottom = (Y_bottom >= MT9_COMPRESS_H-5) ? MT9_COMPRESS_H-5 : Y_bottom;
    Y_top = (Y_top <= 1) ? 1 : Y_top;
    X_right = (X_right >= MT9_COMPRESS_W-10) ? MT9_COMPRESS_W-10 : X_right;
    X_left = (X_left <= 10) ? 10 : X_left;
    // ���·�Ϊ��׼
    // ������ұ߽�
    for (Ysite = Y_top; Ysite <= Y_bottom-2; Ysite++)
    {
        // ��߽�
        if(ImageFrameDetect.frame_left_flag == 0)
        {
            if(mt9_binary_image[Ysite][X_left] == 0 && mt9_binary_image[Ysite+1][X_left] == 1)      ImageFrameDetect.frame_left_flag = 1;    
        }
        else if(ImageFrameDetect.frame_left_flag == 1)
        {
            if(mt9_binary_image[Ysite][X_left] == 1 && mt9_binary_image[Ysite+1][X_left] == 0)      ImageFrameDetect.frame_left_flag = 2;     
        }
        else if(ImageFrameDetect.frame_left_flag == 2)
        {
            if(mt9_binary_image[Ysite][X_left] == 0 && mt9_binary_image[Ysite+1][X_left] == 1)      ImageFrameDetect.frame_left_flag = 3;    
        }
        else if(ImageFrameDetect.frame_left_flag == 3)
        {
            if(mt9_binary_image[Ysite][X_left] == 1 && mt9_binary_image[Ysite+1][X_left] == 0)      ImageFrameDetect.frame_left_flag = 4;
        }
        // �ұ߽�
        if(ImageFrameDetect.frame_right_flag == 0)
        {
            if(mt9_binary_image[Ysite][X_right] == 0 && mt9_binary_image[Ysite+1][X_right] == 1)    ImageFrameDetect.frame_right_flag = 1;
        }
        else if(ImageFrameDetect.frame_right_flag == 1)
        {
            if(mt9_binary_image[Ysite][X_right] == 1 && mt9_binary_image[Ysite+1][X_right] == 0)    ImageFrameDetect.frame_right_flag = 2;   
        }
        else if(ImageFrameDetect.frame_right_flag == 2)
        {
            if(mt9_binary_image[Ysite][X_right] == 0 && mt9_binary_image[Ysite+1][X_right] == 1)    ImageFrameDetect.frame_right_flag = 3;   
        }
        else if(ImageFrameDetect.frame_right_flag == 3)
        {
            if(mt9_binary_image[Ysite][X_right] == 1 && mt9_binary_image[Ysite+1][X_right] == 0)    ImageFrameDetect.frame_right_flag = 4;   
        }
    }
    // ������±߽�
    for (Xsite = X_left; Xsite <= X_right-2; Xsite++)
    {
        // �±߽�
        if(ImageFrameDetect.frame_top_flag == 0)
        {
            if(mt9_binary_image[Y_top][Xsite] == 0 && mt9_binary_image[Y_top][Xsite+1] == 1)        ImageFrameDetect.frame_top_flag = 1;
        }
        else if(ImageFrameDetect.frame_top_flag == 1)
        {
            if(mt9_binary_image[Y_top][Xsite] == 1 && mt9_binary_image[Y_top][Xsite+1] == 0)        ImageFrameDetect.frame_top_flag = 2;     
        }
        else if(ImageFrameDetect.frame_top_flag == 2)
        {
            if(mt9_binary_image[Y_top][Xsite] == 0 && mt9_binary_image[Y_top][Xsite+1] == 1)        ImageFrameDetect.frame_top_flag = 3;
        }
        else if(ImageFrameDetect.frame_top_flag == 3)
        {
            if(mt9_binary_image[Y_top][Xsite] == 1 && mt9_binary_image[Y_top][Xsite+1] == 0)        ImageFrameDetect.frame_top_flag = 4;
        }
        // �ϱ߽�
        if(ImageFrameDetect.frame_bottom_flag == 0)
        {
            if(mt9_binary_image[Y_bottom][Xsite] == 0 && mt9_binary_image[Y_bottom][Xsite+1] == 1)  ImageFrameDetect.frame_bottom_flag = 1;
        }
        else if(ImageFrameDetect.frame_bottom_flag == 1)
        {
            if(mt9_binary_image[Y_bottom][Xsite] == 1 && mt9_binary_image[Y_bottom][Xsite+1] == 0)  ImageFrameDetect.frame_bottom_flag = 2;   
        }
        else if(ImageFrameDetect.frame_bottom_flag == 2)
        {
            if(mt9_binary_image[Y_bottom][Xsite] == 0 && mt9_binary_image[Y_bottom][Xsite+1] == 1)  ImageFrameDetect.frame_bottom_flag = 3;
        }
        else if(ImageFrameDetect.frame_bottom_flag == 3)
        {
            if(mt9_binary_image[Y_bottom][Xsite] == 1 && mt9_binary_image[Y_bottom][Xsite+1] == 0)  ImageFrameDetect.frame_bottom_flag = 4;   
        }
    }
}
void Find_Spinodal(void)  // Ѱ�ҹյ㣨��յ㡢�ҹյ㣩
{
    int i = 0;
    int center_count = 0;  // ����ͳ�����ĵ�ƽ��ֵ
    for (i = 2; i <= 55; i++)
    {
        if(mt9_binary_image[55 - i + 2][1] == 1  && ImageDeal[55 - i + 2].IsLeftFind == 'W' && ImageFrameDetect.spinodal_L_Ysite == 0)
        {
            ImageFrameDetect.spinodal_L_Ysite = 55 - i + 2;
        }
        if(mt9_binary_image[55 - i + 2][78] == 1  && ImageDeal[55 - i + 2].IsRightFind == 'W' && ImageFrameDetect.spinodal_R_Ysite == 0)
        {
            ImageFrameDetect.spinodal_R_Ysite = 55 - i + 2;
        }
        // ȡWǰһ��ΪFirst_W_Point_L�ȣ��ﵽȡһ�е��ȶ����ݵ����ĵ�Ŀ��
        if(ImageDeal[i].IsLeftFind == 'W' && ImageDeal[i+1].IsLeftFind == 'T' && ImageStatus.First_W_Point_L == 0)     ImageStatus.First_W_Point_L = i+1 ;
        if(ImageDeal[i].IsRightFind == 'W' && ImageDeal[i+1].IsRightFind == 'T' && ImageStatus.First_W_Point_R == 0)   ImageStatus.First_W_Point_R = i+1;
        if(ImageDeal[i].IsLeftFind == 'H' && (ImageDeal[i+1].IsLeftFind == 'T' || ImageDeal[i+1].IsLeftFind == 'W') 
        && ImageStatus.First_H_Point_L == 0)    ImageStatus.First_H_Point_L = i+1;
        if(ImageDeal[i].IsRightFind == 'H' && (ImageDeal[i+1].IsRightFind == 'T' || ImageDeal[i+1].IsRightFind == 'W') 
        && ImageStatus.First_H_Point_R == 0)    ImageStatus.First_H_Point_R = i+1;
        if(ImageFrameDetect.spinodal_L_Ysite != 0 && ImageFrameDetect.spinodal_R_Ysite != 0 && ImageStatus.First_W_Point_L != 0 
        && ImageStatus.First_W_Point_R != 0 && ImageStatus.First_H_Point_L != 0 && ImageStatus.First_H_Point_R != 0) break;  // ������ҵ��˾Ϳ�����ǰ����ѭ��
    }
    for(i = 59; i >= ImageStatus.OFFLine; i--)  //���ͼ���ȫ�����ߣ����㻭���ʱ��ȷ��X���꣬���⻭ƫ
    {
        if(ImageDeal[i].IsLeftFind == 'T' && ImageDeal[i].IsRightFind == 'T')
        {
            ImageFrameDetect.image_center_x += ImageDeal[i].Center;
            center_count++;
        }
    }
    ImageFrameDetect.image_center_x /= center_count;
}

void Route_Map_Reset(void)
{
    Route_Map_Last_Index = 0xFF;
    Route_Map_Element_Armed = 1;
    Route_Map_Normal_Frame_Count = 0;
    Route_Map_Rearm_Frame_Count = 0;
    route_expected_road_type = Normol;
    route_raw_road_type = Normol;
    route_fallback_active = 0;
    route_fallback_count = 0;
    route_fallback_distance = 0;
    route_rearm_active = 0;
    route_rearm_frames = 0;
}

static void Route_Map_Set_Type(RoadType_e road_type)
{
    ImageFlag.image_element_angle_flag = 0;
    ImageFlag.image_element_Node_flag = 0;
    ImageFlag.image_element_branch_flag = 0;
    ImageFlag.image_element_cross_flag = 0;

    if(road_type == RightAngle_L || road_type == RightAngle_R)
    {
        ImageFlag.image_element_angle_flag = 1;
    }
    else if(road_type == Node_L || road_type == Node_R || road_type == Node_T)
    {
        ImageFlag.image_element_Node_flag = 1;
    }
    else if(road_type == Branch)
    {
        ImageFlag.image_element_branch_flag = 1;
    }
    else if(road_type == Cross)
    {
        ImageFlag.image_element_cross_flag = 1;
    }

    ImageStatus.Road_type = road_type;
}

static unsigned char Route_Map_Has_Visual_Hint(void)
{
    if(ImageStatus.OFFLine > 2) return 1;
    if(ImageFrameDetect.spinodal_L_Ysite != 0 || ImageFrameDetect.spinodal_R_Ysite != 0) return 1;
    if(ImageStatus.First_W_Point_L != 0 || ImageStatus.First_W_Point_R != 0) return 1;
    if(ImageStatus.First_H_Point_L != 0 || ImageStatus.First_H_Point_R != 0) return 1;
    return 0;
}

static unsigned char Route_Map_Recognition_Ready(void)
{
    if(!route_map_enable || Path_Switch != 0)
    {
        route_rearm_active = 0;
        return 1;
    }
    if(Route_Map_Element_Armed)
    {
        route_rearm_active = 0;
        return 1;
    }

    route_rearm_active = 1;
    if(Route_Map_Rearm_Frame_Count < 255)
    {
        Route_Map_Rearm_Frame_Count++;
    }
    route_rearm_frames = Route_Map_Rearm_Frame_Count;

    if(!Route_Map_Has_Visual_Hint() && ImageStatus.OFFLine <= 2
        && abs(post.gyro_yaw) <= ROUTE_MAP_REARM_MAX_YAW_RATE)
    {
        if(Route_Map_Normal_Frame_Count < ROUTE_MAP_REARM_NORMAL_FRAMES)
        {
            Route_Map_Normal_Frame_Count++;
        }

    }
    else
    {
        Route_Map_Normal_Frame_Count = 0;
    }

    if(Route_Map_Rearm_Frame_Count >= ROUTE_MAP_REARM_MIN_FRAMES
        && encoder.disA >= ROUTE_MAP_REARM_MIN_DISTANCE
        && Route_Map_Normal_Frame_Count >= ROUTE_MAP_REARM_NORMAL_FRAMES)
    {
        Route_Map_Element_Armed = 1;
        Route_Map_Last_Index = 0xFF;
        route_rearm_active = 0;
        return 1;
    }

    return 0;
}

static void Route_Map_Disarm_After_Advance(void)
{
    if(!route_map_enable || Path_Switch != 0) return;

    Route_Map_Element_Armed = 0;
    Route_Map_Normal_Frame_Count = 0;
    Route_Map_Rearm_Frame_Count = 0;
    route_raw_road_type = Normol;
    route_fallback_active = 0;
    route_fallback_distance = 0;
    route_rearm_active = 1;
    route_rearm_frames = 0;
}

static void Route_Map_Apply(void)
{
    uint8_t index;
    RoadType_e raw_type;
    RoadType_e expected_type;
    int fallback_distance;

    if(!route_map_enable || Path_Switch != 0) return;

    index = Path[0].target_angle_index;
    if(index >= Path[0].path_length || index >= 26) return;

    if(Route_Map_Last_Index != index)
    {
        Route_Map_Last_Index = index;
        route_raw_road_type = Normol;
        route_fallback_active = 0;
        route_fallback_distance = 0;
    }

    expected_type = Route_Path0_Expected_Type[index];
    route_expected_road_type = expected_type;
    raw_type = ImageStatus.Road_type;

    if(raw_type != Normol)
    {
        if(route_raw_road_type == Normol) route_raw_road_type = raw_type;
        if(expected_type != Normol && raw_type != expected_type)
        {
            Route_Map_Set_Type(Normol);
            return;
        }
        return;
    }

    fallback_distance = Route_Path0_Fallback_Distance[index];
    if(fallback_distance > 0 && encoder.disA >= fallback_distance
        && Route_Map_Has_Visual_Hint())
    {
        if(index == 3 && encoder.target_distance > 0
            && encoder.disA > encoder.target_distance * 2)
        {
            return;
        }
        route_fallback_active = 1;
        route_fallback_count++;
        route_fallback_distance = encoder.disA;
        Route_Map_Set_Type(expected_type);
    }
}

int Y_top_limit = 5;
int Y_bottom_limit = 55;
float distance_judgment = 0;
void Element_Judgment(void)
{
    // ��ֱ��ת��ʶ��
    if(ImageFrameDetect.spinodal_L_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite == 0
    && ImageFrameDetect.spinodal_L_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_L_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L != 0
    && ImageStatus.First_W_Point_R == 0
    && ImageStatus.First_H_Point_L != 0
    && ImageStatus.First_H_Point_R != 0
    && abs(ImageDeal[ImageStatus.First_W_Point_L+5].Wide - Image_Real_Wide[ImageStatus.First_W_Point_L+5]) <= 5  // ��ױ���������������������
    && abs(ImageFrameDetect.spinodal_L_Ysite - ImageStatus.First_W_Point_L) <= 5
    && ImageFlag.image_element_angle_flag == 0  // ��ֱ��ת��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol
    && Element_lock.RightAngle_L == 1  // ��ֱ��Ԫ���ѽ���
    )  // ��һ����ɫ�ޱߵ�
    {
        // �԰�ɫ�ޱߵ�Ϊ���ģ����м��
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = ImageStatus.First_W_Point_L;
        ImageFrameDetect.frame_width = 50;
        ImageFrameDetect.frame_height = 30;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 0 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 2 && ImageFrameDetect.frame_right_flag == 0)
        {
            ImageFlag.image_element_angle_flag = 1;
            ImageStatus.Road_type = RightAngle_L;  // ʶ��ɹ������ñ�־λ�͵�·����
        }
        else 
        {
            ImageFlag.image_element_angle_flag = 0;
        }
    }
    // ��ֱ��ת��ʶ��
    if(ImageFrameDetect.spinodal_L_Ysite == 0
    && ImageFrameDetect.spinodal_R_Ysite != 0  // ��ɫ�ޱߵ����20
    && ImageFrameDetect.spinodal_R_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_R_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L == 0
    && ImageStatus.First_W_Point_R != 0
    && ImageStatus.First_H_Point_L != 0
    && ImageStatus.First_H_Point_R != 0  // ���Ҳ����жϿ��
    && abs(ImageDeal[ImageStatus.First_W_Point_R+5].Wide - Image_Real_Wide[ImageStatus.First_W_Point_R+5]) <= 5  // ��ױ���������������������
    && abs(ImageFrameDetect.spinodal_R_Ysite - ImageStatus.First_W_Point_R) <= 5  // ���Ҳ����жϿ��
    && ImageFlag.image_element_angle_flag == 0  // ��ֱ��ת��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol
    && Element_lock.RightAngle_R == 1  // ��ֱ��Ԫ���ѽ���
    )
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = ImageStatus.First_W_Point_R;
        ImageFrameDetect.frame_width = 50;
        ImageFrameDetect.frame_height = 30;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 0 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 0 && ImageFrameDetect.frame_right_flag == 2)
        {
            ImageFlag.image_element_angle_flag = 1;
            ImageStatus.Road_type = RightAngle_R;
        }
        else 
        {
            ImageFlag.image_element_angle_flag = 0;
        }
    }
    if(ImageFrameDetect.spinodal_L_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite == 0
    && ImageFrameDetect.spinodal_L_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_L_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L != 0
    && ImageStatus.First_W_Point_R == 0
    && ImageStatus.First_H_Point_L == 0
    && ImageStatus.First_H_Point_R == 0  
    && abs(ImageFrameDetect.spinodal_L_Ysite - ImageStatus.First_W_Point_L) <= 5  
    && ImageFlag.image_element_Node_flag == 0  // ��ڵ�ת��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol
    && Element_lock.Node_L == 1  // ��ڵ�Ԫ���ѽ���
    )
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = ImageStatus.First_W_Point_L;
        ImageFrameDetect.frame_width = 60;
        ImageFrameDetect.frame_height = 40;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 2 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 2 && ImageFrameDetect.frame_right_flag == 0)
        {
            ImageFlag.image_element_Node_flag = 1;
            ImageStatus.Road_type = Node_L;
        }
        else 
        {
            ImageFlag.image_element_Node_flag = 0;
        }
    }
    if(ImageFrameDetect.spinodal_L_Ysite == 0
    && ImageFrameDetect.spinodal_R_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_R_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L == 0
    && ImageStatus.First_W_Point_R != 0
    && ImageStatus.First_H_Point_L == 0
    && ImageStatus.First_H_Point_R == 0  
    && abs(ImageFrameDetect.spinodal_R_Ysite - ImageStatus.First_W_Point_R) <= 5  
    && ImageFlag.image_element_Node_flag == 0  // �ҽڵ�ת��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol
    && Element_lock.Node_R == 1  // �ҽڵ�Ԫ���ѽ���
    )
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = ImageStatus.First_W_Point_R;
        ImageFrameDetect.frame_width = 60;
        ImageFrameDetect.frame_height = 40;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 2 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 0 && ImageFrameDetect.frame_right_flag == 2)
        {
            ImageFlag.image_element_Node_flag = 1;
            ImageStatus.Road_type = Node_R;
        }
        else 
        {
            ImageFlag.image_element_Node_flag = 0;
        }
    }
    if(ImageFrameDetect.spinodal_L_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_R_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L != 0
    && ImageStatus.First_W_Point_R != 0
    && abs(ImageDeal[ImageStatus.First_W_Point_L+5].Wide - Image_Real_Wide[ImageStatus.First_W_Point_L+5]) <= 5  
    && ImageFlag.image_element_Node_flag == 0//T�ͽڵ�ת��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol  // T��·�ڽڵ�Ԫ���ѽ���
    && Element_lock.Node_T == 1//T�ͽڵ�Ԫ���ѽ���
    )
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = (int)((ImageStatus.First_W_Point_L + ImageStatus.First_W_Point_R) / 2);
        ImageFrameDetect.frame_width = 60;
        ImageFrameDetect.frame_height = 40;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 0 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 2 && ImageFrameDetect.frame_right_flag == 2)
        {
            ImageFlag.image_element_Node_flag = 1;
            ImageStatus.Road_type = Node_T;
        }
        else 
        {
            ImageFlag.image_element_Node_flag = 0;
        }
    }
    // �ֲ�·�������
    if(ImageFrameDetect.spinodal_L_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite != 0
    && ImageFrameDetect.spinodal_L_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_L_Ysite <= Y_bottom_limit
    && ImageFrameDetect.spinodal_R_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_R_Ysite <= Y_bottom_limit
    && ImageStatus.First_H_Point_L != 0
    && ImageStatus.First_H_Point_R != 0
    && ImageStatus.First_W_Point_L != 0
    && ImageStatus.First_W_Point_R != 0
    && abs(ImageDeal[ImageStatus.First_W_Point_R+5].Wide - Image_Real_Wide[ImageStatus.First_W_Point_R+5]) > 15  // �ֲ�·�Ĵ�ױ��·�������������Ա��
    && ImageFlag.image_element_branch_flag == 0// �ֲ�·δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol  
    && Element_lock.Branch == 1  // �ֲ�·Ԫ���ѽ���
    )
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = (int)((ImageStatus.First_W_Point_L + ImageStatus.First_W_Point_R) / 2);
        ImageFrameDetect.frame_width = 70;
        ImageFrameDetect.frame_height = 50;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 0 && ImageFrameDetect.frame_bottom_flag == 2 && ImageFrameDetect.frame_left_flag == 2 && ImageFrameDetect.frame_right_flag == 2)
        {
            ImageFlag.image_element_branch_flag = 1;
            ImageStatus.Road_type = Branch;
        }
        else 
        {
            ImageFlag.image_element_branch_flag = 0;
        }
    }
    if(ImageFrameDetect.spinodal_L_Ysite != 0
    && ImageFrameDetect.spinodal_R_Ysite != 0
    && ImageFrameDetect.spinodal_L_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_L_Ysite <= Y_bottom_limit
    && ImageFrameDetect.spinodal_R_Ysite >= Y_top_limit
    && ImageFrameDetect.spinodal_R_Ysite <= Y_bottom_limit
    && ImageStatus.First_W_Point_L != 0
    && ImageStatus.First_W_Point_R != 0
    && ImageStatus.First_H_Point_L == 0
    && ImageStatus.First_H_Point_R == 0
    && abs(ImageDeal[ImageStatus.First_W_Point_L+5].Wide - Image_Real_Wide[ImageStatus.First_W_Point_L+5]) <= 5  // ʮ��·��δʶ��
    && ImageFlag.image_element_cross_flag == 0// ʮ��·��δʶ��
    && encoder.disA >= (int)(encoder.target_distance * distance_judgment)
    && ImageStatus.Road_type == Normol  // ʮ��·��Ԫ���ѽ���
    && Element_lock.Cross == 1// ʮ��·��Ԫ���ѽ���
    )                                            
    {
        ImageFrameDetect.frame_center_x = ImageFrameDetect.image_center_x;
        ImageFrameDetect.frame_center_y = (int)((ImageStatus.First_W_Point_L + ImageStatus.First_W_Point_R) / 2);
        ImageFrameDetect.frame_width = 60;
        ImageFrameDetect.frame_height = 40;
        Frame_Detect(ImageFrameDetect.frame_center_x, ImageFrameDetect.frame_center_y, ImageFrameDetect.frame_width, ImageFrameDetect.frame_height);
        if(ImageFrameDetect.frame_top_flag == 2 && ImageFrameDetect.frame_bottom_flag == 2  && ImageFrameDetect.frame_left_flag == 2 && ImageFrameDetect.frame_right_flag == 2)
        {
            ImageFlag.image_element_cross_flag = 1;
            ImageStatus.Road_type = Cross;
        }
        else 
        {
            ImageFlag.image_element_cross_flag = 0;
        }
    }
}
static void Path_Advance_Node(void)
{
    uint8_t next_index = Path[Path_Switch].target_angle_index;

    if(next_index < Path[Path_Switch].path_length)
    {
        next_index++;
    }
    else
    {
        next_index = Path[Path_Switch].path_length;
    }

    Path[Path_Switch].target_angle_index = next_index;
    encoder.target_distance = Path[Path_Switch].memory_target_distance[next_index];
    Route_Map_Disarm_After_Advance();
}

void Protect_Car(void)
{
    if(ImageStatus.Road_type != Normol)
    {
        race_protect_black_frames = 0;
        return;
    }

    if(ImageStatus.OFFLine >= 55
        && mt9_binary_image[55][MT9_COMPRESS_W/2] == 0
        && mt9_binary_image[55][MT9_COMPRESS_W/3] == 0
        && mt9_binary_image[55][MT9_COMPRESS_W*2/3] == 0)
    {
        if(race_protect_black_frames < 3)
        {
            race_protect_black_frames++;
        }

        if(race_protect_black_frames >= 3)
        {
            race_request_protection_stop(
                ImageStatus.OFFLine,
                mt9_binary_image[55][MT9_COMPRESS_W/3],
                mt9_binary_image[55][MT9_COMPRESS_W/2],
                mt9_binary_image[55][MT9_COMPRESS_W*2/3]);
        }
    }
    else
    {
        race_protect_black_frames = 0;
    }
}
int angle_threshold = 25;
int time = 0;
void Elment_Handle_RightAngle_L(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index+1];  //  ��ֱ��ת�䴦��
    if(ImageStatus.First_W_Point_L < ImageStatus.OFFLine) ImageStatus.First_W_Point_L = ImageStatus.OFFLine;
    if(ImageStatus.First_W_Point_L > 58) ImageStatus.First_W_Point_L = 58;
    for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++)
    {
        ImageDeal[Ysite].Center = 0;
    }  // ��ֱ��ת����ʶ��
    if(ImageFlag.image_element_angle_flag == 1) ImageFlag.image_element_angle_flag = 2;// ��ֱ��ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_angle_flag == 3)
    {
        buzzer_off();
        encoder_clear();
        time = 0;
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_angle_flag = 0;
        Path_Advance_Node();
    }
}
void Elment_Handle_RightAngle_R(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index+1];  // ��ֱ��ת�䴦��
    // ��ֱ��ת�䴦��
    if(ImageStatus.First_W_Point_R < ImageStatus.OFFLine) ImageStatus.First_W_Point_R = ImageStatus.OFFLine;
    if(ImageStatus.First_W_Point_R > 58) ImageStatus.First_W_Point_R = 58;
    for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_R; Ysite++)
    {
        ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;
    }  // ��ֱ��ת����ʶ��
    if(ImageFlag.image_element_angle_flag == 1)     ImageFlag.image_element_angle_flag = 2;// ��ֱ��ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_angle_flag == 3)
    {
        buzzer_off();
        encoder_clear();
        time = 0;
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_angle_flag = 0;
        Path_Advance_Node();
    }
}
void Elment_Handle_Node_L(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    
    // �ڵ㴦��
    if(ImageStatus.First_W_Point_L < ImageStatus.OFFLine) ImageStatus.First_W_Point_L = ImageStatus.OFFLine;
    for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++)
    {  // ��ת����
        if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'L')   ImageDeal[Ysite].Center = 0;  // ��ת����
        else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'R') ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;  // ֱ�п���
        else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'N' && ImageDeal[Ysite].IsLeftFind == 'W') ImageDeal[Ysite].Center = ImageDeal[Ysite].RightBorder -Image_Real_Wide[Ysite] / 2;// ֱ�п���
    }  // ��ڵ�ת����ʶ��
    if(ImageFlag.image_element_Node_flag == 1) ImageFlag.image_element_Node_flag = 2;// ��ڵ�ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_Node_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] != 'N')
    {
        buzzer_off();   
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_Node_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];
    }
    else if(ImageFlag.image_element_Node_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'N' && encoder.disA >= encoder.target_distance)
    {
        buzzer_off();
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_Node_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
}
void Elment_Handle_Node_R(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    // �ڵ㴦��
    if(ImageStatus.First_W_Point_R < ImageStatus.OFFLine) ImageStatus.First_W_Point_R = ImageStatus.OFFLine;
    for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_R; Ysite++) 
    {  // ��ת����
        if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'L')   ImageDeal[Ysite].Center = 0;  // ��ת����
        else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'R') ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;  // ֱ�п���
        else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'N' && ImageDeal[Ysite].IsRightFind == 'W') ImageDeal[Ysite].Center = ImageDeal[Ysite].LeftBorder + Image_Real_Wide[Ysite] / 2;// ֱ�п���
    }  // �ҽڵ�ת����ʶ��
    if(ImageFlag.image_element_Node_flag == 1) ImageFlag.image_element_Node_flag = 2;// �ҽڵ�ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_Node_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] != 'N')
    {
        buzzer_off();   
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_Node_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
    else if(ImageFlag.image_element_Node_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'N' && encoder.disA >= encoder.target_distance)
    {
        buzzer_off();
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_Node_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
}
void Elment_Handle_Node_T(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    // �ڵ㴦��
    if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'L')   
    {  
        if(ImageStatus.First_W_Point_L < ImageStatus.OFFLine) ImageStatus.First_W_Point_L = ImageStatus.OFFLine;
        if(ImageStatus.First_W_Point_L > 58) ImageStatus.First_W_Point_L = 58;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = 0;// ��ת����
        }
    }
    else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'R')
    {  
        if(ImageStatus.First_W_Point_R < ImageStatus.OFFLine) ImageStatus.First_W_Point_R = ImageStatus.OFFLine;
        if(ImageStatus.First_W_Point_R > 58) ImageStatus.First_W_Point_R = 58;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_R; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;// ��ת����
        }
    }  // ��ת�ڵ���ʶ��
    if(ImageFlag.image_element_Node_flag == 1) ImageFlag.image_element_Node_flag = 2;// T�ͽڵ�ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_Node_flag == 3)
    {
        buzzer_off();
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_Node_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
}
void Elment_Handle_Branch(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    // �ڵ㴦��
    if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'L')   
    {
        for(Ysite = ImageStatus.First_W_Point_L; Ysite < 58; Ysite++)
        {
            if(abs(ImageDeal[Ysite].LeftBorder - ImageFrameDetect.image_center_x) <= 5)
            {
                ImageStatus.First_W_Point_L = Ysite;
                break;
            }
        }
        if(ImageStatus.First_W_Point_L < ImageStatus.OFFLine) ImageStatus.First_W_Point_L = ImageStatus.OFFLine;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = 0;// ��ת����
        }
    }
    else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'R')
    {
        for(Ysite = ImageStatus.First_W_Point_R; Ysite < 58; Ysite++)
        {
            if(abs(ImageDeal[Ysite].RightBorder - ImageFrameDetect.image_center_x) <= 5)
            {
                ImageStatus.First_W_Point_R = Ysite;
                break;
            }
        }
        if(ImageStatus.First_W_Point_R < ImageStatus.OFFLine) ImageStatus.First_W_Point_R = ImageStatus.OFFLine;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_R; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;// ��ת����
        }
    }  // �ֲ�·��ʶ��
    if(ImageFlag.image_element_branch_flag == 1) ImageFlag.image_element_branch_flag = 2;// �ֲ�·��ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_branch_flag == 3)
    {
        buzzer_off();
        encoder_clear();
        time = 0;
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_branch_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
}
void Elment_Handle_Cross(void)
{
    buzzer_on();
    post.target_angle = Path[Path_Switch].memory_target_angle[Path[Path_Switch].target_angle_index];
    // �ڵ㴦��
    if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'L')   
    {
        if(ImageStatus.First_W_Point_L < ImageStatus.OFFLine) ImageStatus.First_W_Point_L = ImageStatus.OFFLine;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = 0;// ��ת����
        }
    }
    else if(Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'R')
    {
        if(ImageStatus.First_W_Point_R < ImageStatus.OFFLine) ImageStatus.First_W_Point_R = ImageStatus.OFFLine;
        for(Ysite = 0; Ysite <= ImageStatus.First_W_Point_R; Ysite++)
        {  // ��ת����
            ImageDeal[Ysite].Center = MT9_COMPRESS_W-1;// ��ת����
        }
    }  // �ҽڵ�ת����ʶ��
    if(ImageFlag.image_element_cross_flag == 1) ImageFlag.image_element_cross_flag = 2;// �ҽڵ�ת����ʶ��
    if(abs(post.diff_angle)<angle_threshold   && ImageFlag.image_element_cross_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] != 'N')
    {
        buzzer_off();   
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_cross_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
    else if(ImageFlag.image_element_cross_flag == 3 && Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index] == 'N' && encoder.disA >= encoder.target_distance)
    {
        buzzer_off();
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_cross_flag = 0;
        Path_Advance_Node();
        encoder.target_distance = Path[Path_Switch].memory_target_distance[Path[Path_Switch].target_angle_index];  // �ڵ㴦��
    }
}

void Element_Handle(void)
{
    if(ImageStatus.Road_type == RightAngle_L)
    {
        Elment_Handle_RightAngle_L();
    }
    if(ImageStatus.Road_type == RightAngle_R)
    {
        Elment_Handle_RightAngle_R();
    }
    if(ImageStatus.Road_type == Node_L)
    {
        Elment_Handle_Node_L();
    }
    if(ImageStatus.Road_type == Node_R)
    {
        Elment_Handle_Node_R();
    }
    if(ImageStatus.Road_type == Node_T)
    {
        Elment_Handle_Node_T();
    }
    if(ImageStatus.Road_type == Branch)
    {
        Elment_Handle_Branch();
    }
    if(ImageStatus.Road_type == Cross)
    {
        Elment_Handle_Cross();
    }
}
//�����˲������Խ���ױ߹��˵��������ױߣ����������ó�ȫ������ֵ���ɣ�
static void RouteFilter(void)
{
    if(ImageStatus.Road_type == Normol) 
    {  // �ӿ�ʼλ�õ�ֹͣλ��
        for (Ysite = 58; Ysite >= Ysite_bottom; Ysite--)   // �ӿ�ʼλ�õ�ֹͣλ��
        {
            if(ImageDeal[Ysite].IsLeftFind == 'W' || ImageDeal[Ysite].IsLeftFind == 'H' 
                || ImageDeal[Ysite].IsRightFind == 'W' || ImageDeal[Ysite].IsRightFind == 'H') 
            {
                ImageDeal[Ysite].Center = ImageFrameDetect.image_center_x;
            }
        }
    }
}
// ����ǰհ��Ȩ��
void Get_TowPoint(void)  // ����ǰհ��Ȩ��
{
    TowPoint = 0;
    DetTemp = 0;  // ͨ�����߸߶ȶ�̬�ж�ǰհ
    float WhiteGain = 0;  // ͨ�����߸߶ȶ�̬�ж�ǰհ
    
    //��̬ǰհ
    WhiteGain = 9 * whitehigh_float;    //whitehigh_float = (float)(whitehigh - 40)/(55 - 5)
    if (WhiteGain >= Gain_High)
       WhiteGain = Gain_High * 1.0;
    else if (WhiteGain <= Gain_Low)
       WhiteGain = Gain_Low * 1.0;
    // Ԫ�ع̶�ǰհ
    // ����ԽխǰհԽ��
    TowPoint = ImageStatus.TowPoint - WhiteGain;  // ����ԽխǰհԽ��
    // ��ֹǰհ��̫��ȡ�������ݣ�ȡ�ײ�����Ϊƫ��
    if (TowPoint < ImageStatus.OFFLine)
        TowPoint = ImageStatus.OFFLine + 1;
    // �޷�
    if (TowPoint >= 48) TowPoint = 48;  // �޷�
    else if(TowPoint <= 12) TowPoint = 12;// �޷�
    Ysite_bottom = (TowPoint - 10);
    Ysite_top = (TowPoint + 10);
}
void Get_Deviation(void)
{
    float UnitAll = 0;
    for (int Ysite = Ysite_bottom; Ysite < Ysite_top; Ysite++)
    {
        DetTemp = DetTemp + Weighting_20[Ysite - Ysite_bottom] * (ImageDeal[Ysite].Center);
        UnitAll = UnitAll + Weighting_20[Ysite - Ysite_bottom];
    }
    DetTemp = (ImageDeal[TowPoint].Center + DetTemp) / (UnitAll + 1);

    ImageStatus.Det_True = DetTemp;  // ��ʱ�ļ�Ȩ�����ƽ��ͼ��ƫ��
    ImageStatus.TowPoint_True = TowPoint;   // ��ʱ��ǰհ��
}
void image_data_init(void)
{  // ͼ�����ݳ�ʼ��
    // ������һ�ε�ͼ�񶥱�
    ImageStatus.OFFLine_Last = ImageStatus.OFFLine;  
    ImageStatus.OFFLine = 2;    // ��ʼֵ������ʵ��ʹ��ֵ���е���
    ImageStatus.First_W_Point_L = 0;
    ImageStatus.First_W_Point_R = 0;
    ImageStatus.First_H_Point_L = 0;
    ImageStatus.First_H_Point_R = 0;
    ImageFrameDetect.spinodal_L_Ysite = 0;
    ImageFrameDetect.spinodal_R_Ysite = 0;
    ImageFrameDetect.image_center_x = 0;
    continue_lost_count = 0;
    for (Ysite = 59; Ysite >= ImageStatus.OFFLine; Ysite--)
    {
        ImageDeal[Ysite].IsLeftFind = 'F';
        ImageDeal[Ysite].IsRightFind = 'F';
        ImageDeal[Ysite].LeftBorder = 0;
        ImageDeal[Ysite].RightBorder = 79;
    }
}
// �ϵ�֮��������
void Eye_of_Providence_Task(void)
{  //  �ڴ˴���д�ϵ�֮���������
    // ͼ��copy��ѹ��
    copy_img();
    img_compress();
    img_compress2();
    // ��ѹ�����ͼ�����GammaУ��
    gamma_correction(mt9_compress_image[0], MT9_COMPRESS_W * MT9_COMPRESS_H);  
    Image_Solve_Finish_Flag = 0;  
    image_data_init();  // ͼ�����ݳ�ʼ��
    binary_by_part();
    // ����׷��
    // ���Ƶױ�->�Ȱѵױߵ������߽���Ϣ�ҳ������������Ը�����һ�е���Ϣȥ��
    DrawLinesFirst();  // ��ȡ�������ߣ����ĺ�����
    DrawLinesProcess();  // ��ȡ�������ߣ����ĺ�����
    // Ѱ����İ���
    Fine_whitepoint();  
    // ����ǰհ
    Get_TowPoint();  
    // Ѱ�ҹյ㣬��Ҫ����ȷ��ֱ��ת���ʶ��Χ�Լ�����ʶ������Ԫ��
    Find_Spinodal();  
    // Ԫ��ʶ��
    if((race_state == RACE_STATE_MOTOR_RAMP || race_state == RACE_STATE_RUNNING)
        && Path[Path_Switch].target_angle_index < Path[Path_Switch].path_length)
    {
        if(Route_Map_Recognition_Ready())
        {
            Element_Judgment();
            Route_Map_Apply();
        }
    }
    // �����˲�
    RouteFilter();  
    // Ԫ�ش���
    if(race_state == RACE_STATE_MOTOR_RAMP || race_state == RACE_STATE_RUNNING)
    {
        if(ImageStatus.Road_type != Normol)
        {
            Protect_Last_Road_Type = ImageStatus.Road_type;
            race_protect_black_frames = 0;
        }
        else
        {
            if(Protect_Last_Road_Type != Normol)
            {
                Protect_Last_Road_Type = Normol;
                race_protect_grace_ticks = RACE_PROTECT_EXIT_GRACE_TICKS;
            }

            if(stop_flag == 0 && Element_lock.Protect == 1 && race_protect_grace_ticks == 0)
            {
                Protect_Car();
            }
            else
            {
                race_protect_black_frames = 0;
            }
        }

        if(stop_flag == 0
            && Path[Path_Switch].target_angle_index < Path[Path_Switch].path_length)
        {
            Element_Handle();
        }
    }
    else
    {
        Protect_Last_Road_Type = Normol;
        race_protect_grace_ticks = 0;
        race_protect_black_frames = 0;
    }
    // ����ƫ��
    Get_Deviation();
    Race_Image_Frame_Update();
    // ͼ������ɱ�־λ
    Image_Solve_Finish_Flag = 1;  
}
