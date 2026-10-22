/*
 * Eye_of_Providence.h
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */

#ifndef CODE_EYE_OF_PROVIDENCE_H_
#define CODE_EYE_OF_PROVIDENCE_H_

#define MT9_COMPRESS_W  80
#define MT9_COMPRESS_H  60
#define ImageSensorMid 39                    // ͼ�񴫸����е㣨80�е��е�39��
#define LimitL(L) (L = ((L < 1) ? 1 : L))    // ������߽緶Χ����Сֵ1
#define LimitH(H) (H = ((H > 78) ? 78 : H))  // �����ұ߽緶Χ�����ֵ78


typedef struct {
  int point;
  uint8_t type;
} JumpPointtypedef;

// ����Ԫ������ö��
typedef enum {
  Normol,       // �������� 0
  RightAngle_L,     // ��ֱ�� 1
  RightAngle_R,        // ��ֱ�� 2
  Node_L,   // ��ڵ� 3
  Node_R,  // �ҽڵ� 4
  Node_T,   // T�ͽڵ� 5
  Branch,      // �ֲ�· 6
  Cross,      // ʮ��·�� 7
  Correct,      // ����·�� 8
} RoadType_e;

// Ԫ�������ṹ�壨����ʹ��/���ø�Ԫ��ʶ���Լ�����ʶ����룩
typedef struct {
  uint8_t RightAngle_L;     // ��ֱ��ʹ��
  uint8_t RightAngle_R;        // ��ֱ��ʹ��
  uint8_t Node_L;   // ��ڵ�ʹ��
  uint8_t Node_R;  // �ҽڵ�ʹ��
  uint8_t Node_T;   // T�ͽڵ�ʹ��
  uint8_t Branch;      // �ֲ�·ʹ��
  uint8_t Cross;      // ʮ��·��ʹ��
  uint8_t Correct;      // ����ʹ��
  uint8_t Protect;      // ������־����������ʱ����ͣ������
} RoadType_lock;

// ÿһ�е������߽�����
typedef struct {
    /* ���ұ߽��־: T=��ȷ�ҵ��߽�  W=�ޱ�(��)  H=�ϰ�/����(��)  F=δ��ʼ�� */
    uint8_t IsRightFind;      // �ұ��ҵ��߽��־
    uint8_t IsLeftFind;       // ����ҵ��߽��־
    int Wide;               // �������
    int LeftBorder;         // ��߽�������
    int RightBorder;        // �ұ߽�������
    int LeftBorder2;         // ������߽磨����Ԫ���ж�ʱ������
    int RightBorder2;        // �����ұ߽磨����Ԫ���ж�ʱ������
    int Center;             // ��������������
} ImageDealDatatypedef;

typedef struct {
    /* ���ڹ���ȫ��ͼ����״̬ */
    int TowPoint;             // ǰհ��ʼ�Ƚϵ㣨�кţ�
    int TowPoint_True;        // ʵ��ʹ�õ�ǰհ��
    int Det_True;             // ��GetDeviation()����ļ�Ȩƽ��ͼ��ƫ��
    int Det_Last;             // ��һ�ε�ƫ��ֵ

    uint8_t OFFLine_Last;     // ��һ��ͼ��Ķ��ߣ�������ʼ�У�
    uint8_t OFFLine;          // ��ǰͼ�񶥱ߣ�������ʼ�У�
    uint8_t Left_Line;        // ��߶����к�
    uint8_t Right_Line;       // �ұ߶����к�

    RoadType_e Road_type;  // ��ǰ����Ԫ������

    // Ԫ�ؼ�⸨����
    int First_W_Point_L;        // ��ߵ�һ����ɫ�ޱߵ㣨W�㣩���к�
    int First_W_Point_R;        // �ұߵ�һ����ɫ�ޱߵ㣨W�㣩���к�
    int First_H_Point_L;        // ��ߵ�һ����ɫ�ޱߵ㣨H�㣩���к�
    int First_H_Point_R;        // �ұߵ�һ����ɫ�ޱߵ㣨H�㣩���к�
} ImageStatustypedef;

typedef struct {
    uint8_t image_element_Node_flag;             /* �ڵ����־ */
    uint8_t image_element_angle_flag;             /* ֱ�Ǽ���־ */
    uint8_t image_element_branch_flag;             /* �ֲ�·����־ */
    uint8_t image_element_cross_flag;             /* ʮ��·�ڼ���־ */
} ImageFlagtypedef;// ͼ��Ԫ��ʶ���־λ�ṹ��

typedef struct{
    int spinodal_L_Ysite;             // ��յ��кţ�W������ʼ�У�
    int spinodal_R_Ysite;             // �ҹյ��кţ�W������ʼ�У�

    int image_center_x;             // ͼ�����ĵ�������X����Ȩƽ����
    int frame_center_x;             // ��������������X
    int frame_center_y;             // ��������������Y
    int frame_width;                // ������
    int frame_height;               // ����߶�

    int frame_bottom_flag;             /* �����±߽�ڰ������־ */
    int frame_right_flag;             /* �����ұ߽�ڰ������־ */
    int frame_top_flag;             /* �����ϱ߽�ڰ������־ */
    int frame_left_flag;             /* ������߽�ڰ������־ */
} ImageFrameDetecttypedef;// ͼ��֡���ṹ�壨����Ԫ��ʶ��ʱ�ı߽��⣩

typedef struct{
    int memory_target_angle[45];                 /* Ŀ��Ƕ����� */
    int memory_target_distance[45];                 /* Ŀ��������� */
    uint8_t memory_target_angle_turn[45];                 /* Ŀ��ת�䷽���־��L��/R��/Nֱ�У� */
    uint8_t target_angle_index;             /* ��ǰĿ��Ƕ����� */
    uint8_t path_length;             /* ·���յ㳤�ȣ�Ԫ�ظ����� */
} Pathtypedef;// ·���ṹ�壨�洢Ԥ��·���㣩

//------------------------------------------------------------------------------------------------------------------//

//------------------------------------------- ȫ�ֱ������� ---------------------------------------------//
extern ImageStatustypedef ImageStatus;               // ͼ����״̬
extern ImageDealDatatypedef ImageDeal[60];           // ��¼ÿ�е������߽���Ϣ
extern ImageFlagtypedef ImageFlag;                   // ͼ��Ԫ��ʶ���־λ
extern RoadType_lock Element_lock;                   // Ԫ��������ʹ�ܿ��ƣ�
extern Pathtypedef Path[3];                          // ·�����飨����Ԥ��·����
extern ImageFrameDetecttypedef ImageFrameDetect;     // ͼ��֡���ṹ��

extern uint8_t mt9_compress_image[MT9_COMPRESS_H][MT9_COMPRESS_W];     // ѹ�����ͼ������
extern uint8_t mt9_copy_image[MT9V03X_1_H][MT9V03X_1_W - 8];          // ���ƺ��ԭʼͼ������
extern uint8_t mt9_image_show[MT9_COMPRESS_H][MT9_COMPRESS_W];         // ���ڲ˵���ʾ��ͼ������
extern uint8_t mt9_binary_image[MT9_COMPRESS_H][MT9_COMPRESS_W];       // ��ֵ�����ͼ������
extern uint8_t mt9_binary_image_wifi_show[MT9_COMPRESS_H][MT9_COMPRESS_W]; // WiFi�����õĶ�ֵ��ͼ��
extern uint8_t whitehigh;          // ���߸߶ȣ��ӵ������ϰ�����������
extern int threshold_far;          // Զ�˶�ֵ����ֵ
extern int threshold_middle;       // �м��ֵ����ֵ
extern int threshold_near;         // ���˶�ֵ����ֵ
extern int threshold_total;        // ȫ�ִ����ֵ
extern uint8_t OSTU_Low;           // �����ֵ����
extern uint8_t OSTU_High;          // �����ֵ����
extern int Gain_High;              // ǰհ��������
extern int Gain_Low;               // ǰհ��������
extern int TowPoint;               // ��ǰǰհ���к�
extern float DetTemp;              // ƫ����ʱ����ֵ
extern int Err;                    // ���ֵ
extern int ImageScanInterval_small; // С��Χɨ��������
extern int continue_lost_count;    // �������߼���
extern uint8_t Image_Solve_Finish_Flag; // ͼ������ɱ�־
extern float distance_judgment;    // Ԫ��ʶ�����ϵ��
extern int angle_num;              // ת�Ǽ���
extern float gamma_value;          // GammaУ������
extern int angle_threshold;        // element completion angle threshold
//------------------------------------------- ȫ�ֱ������� ---------------------------------------------//


extern int route_map_enable;
extern int route_approach_speed;
extern volatile int route_expected_road_type;
extern volatile int route_raw_road_type;
extern volatile int route_fallback_active;
extern volatile int route_fallback_count;
extern volatile int route_fallback_distance;
extern volatile int route_rearm_active;
extern volatile int route_rearm_frames;

void copy_img(void);
void img_compress(void);
void img_compress2(void);

void Eye_of_Providence_Task(void);
void Fine_whitepoint(void);
void Get_Deviation(void);
void GetJumpPointFromDet(uint8_t* p,uint8_t type,int L,int H,JumpPointtypedef* JumpPoint);
void Get_Actually_L_R_Line(int row, uint8_t type,JumpPointtypedef* JumpPoint);
void binary_by_part(void);
int Dajin_of_part(uint8_t *img_data, uint32_t pixel_count);
static void DrawLinesFirst(void);
static void RouteFilter(void);// ·���˲����Զ�������ͼ���������
static void DrawLinesProcess(void);
void gamma_init(float gamma);
void gamma_correction(unsigned char* img, int size);
void Element_Judgment(void);
void Element_Handle(void);
void Elment_Handle_RightAngle_L(void);
void Elment_Handle_RightAngle_R(void);
void Elment_Handle_Cross(void);
void Elment_Handle_Node_L(void);
void Elment_Handle_Node_R(void);
void Elment_Handle_Node_T(void);
void Elment_Handle_Branch(void);
void image_data_init(void);
void Protect_Car(void);
void Get_TowPoint(void);
void Frame_Detect(int center_x, int center_y, int frame_width, int frame_height);


void Route_Map_Reset(void);
void Race_Image_Reset(void);
void Race_Image_Frame_Update(void);
void Race_Image_Transmit_Update(void);

#endif /* CODE_EYE_OF_PROVIDENCE_H_ */
