
// MDSSpinnakerSDKDlg.h: 헤더 파일
//

#pragma once
#include "Radiometric_Utility.h"
#include "CMMTiming.h"

// CMDSSpinnakerSDKDlg 대화 상자
class CMDSSpinnakerSDKDlg : public CDialogEx
{
// 생성입니다.
public:
	CMDSSpinnakerSDKDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CMDSSpinnakerSDKDlg();
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MDSSPINNAKERSDK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

public:
	CameraPtr m_Device;
	CNodePtr m_Node;
	SystemPtr m_System;
	CameraList m_CamList;
	CameraModelList m_CamModelList;
	TPConstants* m_tauPlanckConstants;
	ObjParams* m_objectParams;
	stRParams* m_spectralResponseParams;

public:
	CListBox m_CamListBox;
	CListBox m_LogListBox;
	CComboBox m_cbColormap;
	CComboBox m_cbFPS;
	CComboBox m_cbIRFormat;
	CComboBox m_cbTempRange;
	CButton m_radio;
	CButton m_SEQSave;

	CBrush m_bRed;
	CBrush m_bGreen;
	COLORREF m_Color1, m_Color2;
public:
	int m_nSelCamIndex;
	int m_nHeight;
	int m_nWidth;
	bool m_bMeasCapable;
	CString m_strPixelFormat = _T("");
	PaletteTypes m_colormapType; 
	CStatic m_lbLive;

protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	std::queue<ImagePtr> imageQueue;
	std::mutex queueMutex;
	std::mutex bufferMutex;  // 버퍼에 대한 접근을 동기화하기 위한 뮤텍스
	std::condition_variable queueCond;
	std::atomic<bool> m_PauseStreaming;  // 스트리밍 일시 정지 상태
	HBRUSH m_hbrBackground;
	Radiometric_Utility* m_RadUtill;
	CMMTiming mm_timer;     // MM 타이머 객체
	std::queue<ImagePtr> bufferQueue; // 이미지 데이터 버퍼를 저장하는 큐
	std::condition_variable bufferNotEmpty; // 버퍼 대기

	CWinThread* pThread;
	CWinThread* pThreadConsumer;

	std::thread imageProcessingThread;
	std::atomic<bool> ImgProcThreadRunning;
	ThreadStatus m_TStatus; // 스레드 상태
	std::unique_ptr<BITMAPINFO> m_BitmapInfo;
	cv::Mat m_CurrentImageMat; // 현재 처리 중인 이미지를 저장하는 멤버 변수
	PaletteManager paletteManager;
	std::vector<int64_t> m_IRFrameRates; // IR 프레임 레이트를 저장할 벡터
	std::vector<CString> m_IRFrameRateNames; // IR 프레임 레이트 이름을 저장할 벡터
	//QUERY
	int m_nCurrentCase[QUERYCOUNT] = { 0 };
	double m_dQueryCaseLowLimit[QUERYCOUNT] = { 0 };
	double m_dQueryCaseHighLimit[QUERYCOUNT] = { 0 };
	int m_nQueryCnt = 0;
	CameraIRFormat m_IRFormat;
	cv::Rect m_roi_rect;
	// Box 영역 내의 최대, 최소 온도값
	ushort m_Max = 0;
	ushort m_Min = 65535;

	int m_Max_X;
	int m_Max_Y;
	int m_Min_X;
	int m_Min_Y;

	MDSMeasureMaxSpotValue m_MaxSpot; // 최대 스팟 값
	MDSMeasureMinSpotValue m_MinSpot; // 최소 스팟 값

	//SEQ
	bool m_isRecording; // 녹화 중 여부
	bool m_bStartRecording; // 녹화 시작 여부
	bool m_bSaveAsSEQ; // SEQ 파일 저장 여부
	int m_iCurrentSEQImage;
	CFile m_SEQFile;
	DWORD m_TrigCount;
	DWORD m_LineState1;
	DWORD m_LineState2;
	DWORD m_TrigCount1;
	DWORD m_TrigCount2;
	ULONG m_Level;
	ULONG m_Span;
	time_t m_ts;
	int m_ms;
	short m_tzBias;
	std::string m_SEQfilePath; // SEQ 파일 경로
	//
	CString SetFolderPath(CString _str);
	bool m_blink;
	bool m_bMouseImageSave;

public:

	void StartThreadProc();
	static UINT AFX_CDECL ThreadCam(LPVOID _mothod); // 카메라 스레드
	void ConsumerThread();
	static UINT AFX_CDECL ConsumerThreadProc(LPVOID param);
	void DataProcessing(byte* buffer);
	void RenderDataSequence(byte* imageDataPtr);
	std::unique_ptr<uint16_t[]> extractWholeImage(const uint8_t* byteArr, int byteArrLength, int nWidth, int nHeight);
	CString ConvertIPToString(int ipValue);
	void InitVariables();
	void InitCamlist();
	void Camera_destroy();
	CameraPtr ConnectToDevice(int nIndex);
	void CameraStart();
	void DisConnectToDevice();
	bool AcquireImages(CameraPtr pCam);
	bool CameraParamSetting(CameraPtr pCam);
	CameraModelList FindCameraModel(INodeMap* lDeviceParams);
	bool SetStreamingCameraParameters(INodeMap* lDeviceParams);
	CString CMDSSpinnakerSDKDlg::DeterminePixelFormat(const CString& pixelFormatCode);
	DWORD ConvertHexValue(const CString& hexString);
	void AddLog(LPCTSTR lpszFormat, ...);
	void CreateHorizontalScroll();
	std::string GetRootPathA();
	bool SetAutoFocus();
	bool SetNUC();
	int FocusPos(CameraPtr pCam);
	int FocusMethod(CameraPtr pCam,int methodIndex);
	double CameraFPS(CameraPtr pCam);
	bool GetTempRangeSearch(CameraPtr pCam);
	bool GetIRFrameRates(CameraPtr pCam);
	bool SetTempRange(int nQueryIndex);
	CameraIRFormat GetIRFormat();
	void SetIRFormat(CameraIRFormat IRFormat);
	CameraModelList GetCameraModelList();
	double GetScaleFactor();
	bool IsRoiValid(const cv::Rect& roi, int imageWidth, int imageHeight);
	std::unique_ptr<uint16_t[]> extractROI(const uint8_t* byteArr, int nWidth, int nStartX, int nEndX, int nStartY, int nEndY, int roiWidth, int roiHeight);
	void ProcessImageData(std::unique_ptr<uint16_t[]>&& data, int size, int nImageWidth, int nImageHeight, cv::Rect roi);
	bool InitializeTemperatureThresholds();
	void ROIXYinBox(ushort uTempValue, double dScale, int nCurrentX, int nCurrentY, cv::Rect roi, int nPointIdx);
	void UpdateCalcParams();
	void UpdateDeviceOP();
	void DrawRoiRectangle(cv::Mat& image);
	void SetPauseStreaming(bool pause);
	bool GetPauseStreaming() const;
	CString GetLocalIPAddress();
	unsigned long ConvertIPStringToNum(const CString& ipStr);
	std::vector<CString> SplitIP(const CString& ip);
	bool IsThirdOctetDifferent(const CString& ip1, const CString& ip2);
	bool SetCameraIP(CameraPtr pCam, const CString& newIP);
	void AdjustCameraIPToMatchLocalNetwork(CameraPtr pCam, const CString& localIP);
	void FindCamera();
	void StartProc();
	bool IsSameNetwork(const CString& ip1, const CString& ip2);
	CString GetLocalNetworkBaseAddress(const CString& cameraIP);
	CString str2CString(const char* _str);
	std::string CString2str(CString _str);

	template <typename T>
	cv::Mat NormalizeAndProcessImage(const T* data, int height, int width, int cvType);
	cv::Mat DisplayLiveImage(cv::Mat& processedImageMat, bool bGrayType);
	void CleanupAfterProcessing();
	BITMAPINFO* CreateBitmapInfo(const cv::Mat& imageMat);
	void CreateAndDrawBitmap(const cv::Mat& imageMat);
	void DrawImage(Mat mImage, int nID, BITMAPINFO* BitmapInfo);
	LRESULT OnUpdateImage(WPARAM wParam, LPARAM lParam);
	LRESULT OnDisplayImage(WPARAM wParam, LPARAM lParam);
	cv::Mat MapColorsToPalette(const cv::Mat& inputImage, PaletteTypes palette);
	cv::Mat applyIronColorMap(cv::Mat& im_gray, PaletteTypes palette);
	std::vector<cv::Vec3b> adjustPaletteScale(const std::vector<cv::Vec3b>& originalPalette);
	cv::Mat CreateLUT(const std::vector<cv::Vec3b>& adjusted_palette);
	void SetPaletteType(PaletteTypes type); // 팔레트 타입 설정
	PaletteTypes GetPaletteType(); // 팔레트 타입 반환
	PaletteTypes GetSelectedColormap(CComboBox& comboControl);
	void ApplyColorSettings(PaletteTypes selectedMap);
	void PopulateComboBoxes();
	void PopulateComboBoxes_Params();
	bool ValidateRoiValue(int value, int maxValue, const CString& valueName);
	int GetRoiValue(int controlId);
	int GetSelectedComboBoxNumber(int controlId);
	void deleteVariables();
	void ClearQueue();
	void AddBufferToQueue(ImagePtr buffer);
	ImagePtr GetBufferFromQueue();
	int GetQueueBufferCount();
	ImagePtr CopyBuffer(ImagePtr originalBuffer);
	int UpdateHeightForCamera(int nHeight, int nWidth);
	bool FFF_HeightSummary(CameraPtr pCam);
	void UpdateTriggerCountsAndLineStates(std::unique_ptr<uint16_t[]>&& data, int nWidth, int nHeight);

	// SEQ 녹화 메서드
	bool StartSEQRecording(CString strfilePath); // SEQ 녹화 시작
	void SaveSEQ(int nWidth, int nHeight); // SEQ 저장
	void SetStartRecordingFlag(bool bFlag); // 녹화 시작 플래그 설정
	bool GetStartRecordingFlag(); // 녹화 시작 플래그 반환
	void StopRecording(); // 녹화 중지
	bool StartRecording(int frameWidth, int frameHeight, double frameRate); // 녹화 시작
	void RecordThreadFunction(double frameRate); // 녹화 쓰레드 함수
	//
	void SetMouseImageSaveFlag(bool bFlag);
	bool GetMouseImageSaveFlag();
	bool IsMouseEventCheck(UINT message);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ShowPopupMenu(CPoint point);
	void OnPopupMenuSave();
	bool SaveImage(const cv::Mat& image);
	std::string GenerateFileNameWithTimestamp(const std::string& basePath, const std::string& prefix, const std::string& extension);

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnAddLog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnDisconnect();
	afx_msg void OnLbnSelchangeCamList();
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnSeq();
	afx_msg void OnCbnSelchangeCbColormap();
	afx_msg void OnCbnSelchangeCbFps();
	afx_msg void OnCbnSelchangeCbTempRange();
	afx_msg void OnCbnSelchangeCbIrFormat();
	afx_msg void OnBnClickedBtnAf();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnRoiSet();
	afx_msg void OnBnClickedBtnNuc();
	afx_msg void RadioCtrl(UINT radio_Index);
	afx_msg void OnBnClickedBtnChangeip();
};
