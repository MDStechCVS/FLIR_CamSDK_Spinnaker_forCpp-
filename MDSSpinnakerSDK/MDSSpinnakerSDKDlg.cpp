﻿
// MDSSpinnakerSDKDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "MDSSpinnakerSDK.h"
#include "framework.h"
#include "MDSSpinnakerSDKDlg.h"
#include "afxdialogex.h"
#include "Radiometric_Utility.h"
#include "temperature.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#define REMOTE_DEBUG	0
#endif

const TCHAR* MDS_IRFormatArray::MDS_IRFormatStrings[] =
{
	_T("RADIOMETRIC"),
	_T("TEMPERATURELINEAR10MK"),
	_T("TEMPERATURELINEAR100MK"),
	nullptr
};

// =============================================================================
// CMDSSpinnakerSDKDlg 대화 상자
CMDSSpinnakerSDKDlg::CMDSSpinnakerSDKDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MDSSPINNAKERSDK_DIALOG, pParent)
	,m_hbrBackground(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// =============================================================================
CMDSSpinnakerSDKDlg::~CMDSSpinnakerSDKDlg()
{
	if (m_hbrBackground != NULL)
	{
		::DeleteObject(m_hbrBackground);
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CAM_LIST, m_CamListBox);
	DDX_Control(pDX, IDC_LOG, m_LogListBox);
	DDX_Control(pDX, IDC_CB_COLORMAP, m_cbColormap);
	DDX_Control(pDX, IDC_CB_TEMP_RANGE, m_cbTempRange);
	DDX_Control(pDX, IDC_CB_FPS, m_cbFPS);
	DDX_Control(pDX, IDC_CB_IR_FORMAT, m_cbIRFormat);
	DDX_Control(pDX, IDC_BTN_SEQ, m_SEQSave);
	DDX_Control(pDX, IDC_LIVE, m_lbLive);
}

BEGIN_MESSAGE_MAP(CMDSSpinnakerSDKDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_START, &CMDSSpinnakerSDKDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CMDSSpinnakerSDKDlg::OnBnClickedBtnDisconnect)
	ON_LBN_SELCHANGE(IDC_CAM_LIST, &CMDSSpinnakerSDKDlg::OnLbnSelchangeCamList)
	ON_MESSAGE(WM_ADDLOG, &CMDSSpinnakerSDKDlg::OnAddLog)
	ON_MESSAGE(WM_UPDATE_IMAGE, &CMDSSpinnakerSDKDlg::OnUpdateImage)
	ON_MESSAGE(WM_DISPLAY_IMAGE, &CMDSSpinnakerSDKDlg::OnDisplayImage)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_SEQ, &CMDSSpinnakerSDKDlg::OnBnClickedBtnSeq)
	ON_CBN_SELCHANGE(IDC_CB_COLORMAP, &CMDSSpinnakerSDKDlg::OnCbnSelchangeCbColormap)
	ON_CBN_SELCHANGE(IDC_CB_FPS, &CMDSSpinnakerSDKDlg::OnCbnSelchangeCbFps)
	ON_CBN_SELCHANGE(IDC_CB_TEMP_RANGE, &CMDSSpinnakerSDKDlg::OnCbnSelchangeCbTempRange)
	ON_CBN_SELCHANGE(IDC_CB_IR_FORMAT, &CMDSSpinnakerSDKDlg::OnCbnSelchangeCbIrFormat)
	ON_BN_CLICKED(IDC_BTN_AF, &CMDSSpinnakerSDKDlg::OnBnClickedBtnAf)
	ON_BN_CLICKED(IDC_BTN_STOP, &CMDSSpinnakerSDKDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_ROI_SET, &CMDSSpinnakerSDKDlg::OnBnClickedBtnRoiSet)
	ON_BN_CLICKED(IDC_BTN_NUC, &CMDSSpinnakerSDKDlg::OnBnClickedBtnNuc)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_FOCUS_COARSE, IDC_FOCUS_FINE, &CMDSSpinnakerSDKDlg::RadioCtrl)
	ON_BN_CLICKED(IDC_BTN_CHANGEIP, &CMDSSpinnakerSDKDlg::OnBnClickedBtnChangeip)
	ON_COMMAND(ID_POPUP_MENUITEM1, &CMDSSpinnakerSDKDlg::OnPopupMenuSave)
END_MESSAGE_MAP()

// =============================================================================
// CMDSSpinnakerSDKDlg 메시지 처리기
BOOL CMDSSpinnakerSDKDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_hbrBackground = ::CreateSolidBrush(RGB(255, 255, 255));
	m_Color1 = RGB_RED;
	m_Color2 = RGB_GREEN;
	m_bRed.CreateSolidBrush(m_Color1);
	m_bGreen.CreateSolidBrush(m_Color2);

	RadioCtrl(IDC_FOCUS_COARSE);
	// Palette 생성
	std::string baseDir = GetRootPathA() + "\\res\\palette";
	paletteManager.init(baseDir);
	AddLog(_T(" Palette Loading Complated"));

	PopulateComboBoxes();
	InitCamlist();
	m_CamListBox.SetCurSel(0);
	// GUI타이머 시작
	SetTimer(TIMER_ID_GUI_UPDATE, 1000, NULL);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

bool CMDSSpinnakerSDKDlg::IsMouseEventCheck(UINT message)
{
	return (message == WM_LBUTTONDOWN || message == WM_MOUSEMOVE || message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK);
}

BOOL CMDSSpinnakerSDKDlg::PreTranslateMessage(MSG* pMsg)
{
	CStatic* DisplayCam = (CStatic*)GetDlgItem(IDC_CAM);

	if (pMsg->message == WM_RBUTTONDOWN)
	{
		if(m_TStatus == THREAD_STATUS::THREAD_RUNNING)
		{
			CPoint point(pMsg->pt);
			ScreenToClient(&point);

			CRect rect;
			DisplayCam->GetWindowRect(&rect);
			// rect를 클라이언트 좌표로 변환합니다.
			ScreenToClient(&rect);

			if (rect.PtInRect(point))
			{
				// 팝업 메뉴를 올바른 위치에 표시하기 위해 좌표를 다시 화면 좌표로 변환합니다.
				CPoint screenPoint = point;
				ClientToScreen(&screenPoint);

				ShowPopupMenu(screenPoint);
			}
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMDSSpinnakerSDKDlg::ShowPopupMenu(CPoint point)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_POPUP_MENUITEM1, _T("Overlay Image Save"));

	// Menu event mapping
	CWnd* pWnd = GetForegroundWindow();

	if (pWnd != nullptr)
	{
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWnd);
	}

	menu.DestroyMenu();
}

// 핸들러 함수 정의
void CMDSSpinnakerSDKDlg::OnPopupMenuSave()
{
	// 추가적인 처리 코드 작성
	AddLog(_T("Save Overlay Image"));
	SetMouseImageSaveFlag(true);
}

// =============================================================================
// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.
void CMDSSpinnakerSDKDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// =============================================================================
// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMDSSpinnakerSDKDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);

	if (nIDEvent == TIMER_ID_GUI_UPDATE)
	{
		UpdateCalcParams();
		UpdateDeviceOP();
		
		if (GetStartRecordingFlag())
		{
			m_blink = !m_blink;
			std::this_thread::sleep_for(std::chrono::milliseconds(0));


			m_lbLive.Invalidate();
			m_lbLive.UpdateWindow();
		}
	}
}

// =============================================================================
std::string CMDSSpinnakerSDKDlg::GetRootPathA()
{
	// ANSI 형식의 루트 경로를 가져오는 함수.
	char path[MAX_PATH] = { 0, };
	::GetModuleFileNameA(NULL, path, MAX_PATH);

	std::string::size_type pos = std::string(path).find_last_of("\\");

	return std::string(path).substr(0, pos);
}

// =============================================================================
CString CMDSSpinnakerSDKDlg::ConvertIPToString(int ipValue)
{
	// 각 옥텟을 계산하고 문자열로 변환
	CString ipStr;
	ipStr.Format(_T("%d.%d.%d.%d"),
		(ipValue >> 24) & 0xFF, // 첫 번째 옥텟
		(ipValue >> 16) & 0xFF, // 두 번째 옥텟
		(ipValue >> 8) & 0xFF,  // 세 번째 옥텟
		ipValue & 0xFF);        // 네 번째 옥텟
	return ipStr;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::InitVariables()
{
	m_RadUtill = new Radiometric_Utility;

	SetIRFormat(CameraIRFormat::RADIOMETRIC);

	m_nSelCamIndex = 0;
	m_bMeasCapable = false;

	m_tauPlanckConstants = new TPConstants;
	m_objectParams = new ObjParams;
	m_spectralResponseParams = new stRParams;
	m_strPixelFormat = "0x1100007"; // Mono 16
	//m_strPixelFormat = "0x1080001"; // Mono 8

	m_objectParams->AtmTemp = m_objectParams->ExtOptTemp = m_objectParams->AmbTemp = 293.15f;
	m_objectParams->ExtOptTransm = 1.0f; // default
	m_objectParams->ObjectDistance = 2.0; // Default
	m_objectParams->RelHum = 0.5; // Default
	m_objectParams->Emissivity = 1.0; // Default

	m_tauPlanckConstants->J1 = 1;
	m_tauPlanckConstants->J0 = 0;

	// Initiate spectral response
	m_spectralResponseParams->X = 1.9;
	m_spectralResponseParams->alpha1 = 0.006569;
	m_spectralResponseParams->beta1 = -0.002276;
	m_spectralResponseParams->alpha2 = 0.01262;
	m_spectralResponseParams->beta2 = -0.00667;

	SetDlgItemText(IDC_ED_ROI_X, _T("0"));
	SetDlgItemText(IDC_ED_ROI_Y, _T("0"));

	//Defalut
	m_roi_rect.x = 0;
	m_roi_rect.y = 0;

	m_TrigCount = 0;
	m_TrigCount1 = 0;
	m_TrigCount2 = 0;
	m_LineState1 = 0;
	m_LineState2 = 0;

	m_bStartRecording = false;

	CString strLog = _T("");
	strLog.Format(_T("CameraParams Variable Initialize "));
	AddLog(strLog);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::deleteVariables()
{
	delete m_RadUtill;
	m_RadUtill = nullptr;

	delete m_tauPlanckConstants;
	m_tauPlanckConstants = nullptr;

	delete m_objectParams;
	m_objectParams = nullptr;

	delete m_spectralResponseParams;
	m_spectralResponseParams = nullptr;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::Camera_destroy()
{
	DisConnectToDevice();
}

void CMDSSpinnakerSDKDlg::FindCamera()
{
	m_System = System::GetInstance();
	m_CamList = m_System->GetCameras();
	m_TStatus = ThreadStatus::THREAD_IDLE;

	CString localIP = GetLocalIPAddress();
	CameraPtr pCam = nullptr;
	CString Message;
	// 시스템에 있는 카메라 리스트들 가지고온다
	for (unsigned int i = 0; i < m_CamList.GetSize(); i++)
	{
		pCam = m_CamList.GetByIndex(i);
		INodeMap& nodeMap = pCam->GetTLDeviceNodeMap();
		CIntegerPtr ptrIP = nodeMap.GetNode("GevDeviceIPAddress");
		CStringPtr ptrModelName = nodeMap.GetNode("DeviceModelName");

		if (IsReadable(ptrIP) && IsReadable(ptrModelName))
		{
			int64_t nValue = ptrIP->GetValue();
			CString ipStr = ConvertIPToString(nValue);
			CString modelName(CA2CT(ptrModelName->GetValue().c_str()));

			// IP 주소와 디바이스 이름을 로그 메시지로 포맷
			Message.Format(_T("Camera Index: [%d] Model Name: [%s], IP: [%s]"), i + 1, modelName, ipStr);


			// 세 번째 옥텟이 다를 경우 메시지에 추가
		/*	if (IsThirdOctetDifferent(ipStr, localIP))
			{
				Message.Append(_T(" - Different third octet"));
			}*/

			// 리스트박스에 메시지 추가
			m_CamListBox.AddString(Message);
		}
	}
}
// =============================================================================
void CMDSSpinnakerSDKDlg::InitCamlist()
{
	InitVariables();
	FindCamera();
}

// =============================================================================
void CMDSSpinnakerSDKDlg::DisConnectToDevice()
{
	CString logMessage;
	CameraPtr pCam = nullptr;

	if (m_Device == nullptr)
		return;

	// 카메라 선택하지 않았으면 리턴
	if (m_nSelCamIndex == -1)
		return;

	m_TStatus = ThreadStatus::THREAD_IDLE;
	AddLog(_T("------------------------------------"));
	try
	{
		if (m_Device->IsStreaming())
		{
			// 스트리밍 중단
			m_Device->EndAcquisition();
			AddLog(_T("Camera EndAcquisition"));
			// 사용 중인 모든 이미지 데이터를 해제
			while (!imageQueue.empty())
			{
				ImagePtr pImage = imageQueue.front();
				imageQueue.pop();
				pImage->Release();
			}
		}
		SetPauseStreaming(false);
		m_Device->DeInit();
		AddLog(_T("Camera DeInit"));
		m_Device = nullptr;

	}
	catch (Spinnaker::Exception& e)
	{
		logMessage.Format(_T("Error : %S"), e.what());
		AddLog(logMessage);
	}
	AddLog(_T("------------------------------------"));
}

// =============================================================================
CameraPtr CMDSSpinnakerSDKDlg::ConnectToDevice(int nIndex)
{
	CString logMessage;
	CameraPtr pCam = nullptr;
	
	try
	{
		// 카메라 선택하지 않았으면 리턴

		if (nIndex == -1)
			return nullptr;
		
		pCam = m_CamList.GetByIndex(nIndex);
		pCam->Init();
		logMessage.Format(_T("Camera Init Success"));
		AddLog(logMessage);

	}
	catch (Spinnaker::Exception& e)
	{
		std::cout << "Error: " << e.what() << endl;
		logMessage.Format(_T("Error : %S"), e.what());
		AddLog(logMessage);
	}

	AddLog(_T("------------------------------------"));

	return pCam;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::SetStreamingCameraParameters(INodeMap* lDeviceParams)
{
	bool bFlag = false;
	CameraIRFormat nIRFormat = GetIRFormat();
	CameraModelList CamList = GetCameraModelList();
	CString strIRType = _T("");
	CString logMessage;

	switch(CamList)
	{
	case CAM_MODEL::FT1000:
	case CAM_MODEL::XSC:
	case CAM_MODEL::A300:
	case CAM_MODEL::A400:
	case CAM_MODEL::A500:
	case CAM_MODEL::A700:
	case CAM_MODEL::A615:
	case CAM_MODEL::A50:

		CEnumerationPtr ptrIRMode = lDeviceParams->GetNode("IRFormat");
		CEnumEntryPtr IR_Entry = nullptr;
		// 0 == RADIOMETRIC
		// 1 == TemperatureLinear10mK
		// 2 == TemperatureLinear100mK

		if (nIRFormat == Camera_IRFormat::TEMPERATURELINEAR10MK)
		{
			strIRType.Format(_T("TemperatureLinear10mK"));
			IR_Entry = ptrIRMode->GetEntryByName("TemperatureLinear10mK");
		}
		else if (nIRFormat == Camera_IRFormat::TEMPERATURELINEAR100MK)
		{
			strIRType.Format(_T("TemperatureLinear100mK"));
			IR_Entry = ptrIRMode->GetEntryByName("TemperatureLinear100mK");
		}
		else if (nIRFormat == Camera_IRFormat::RADIOMETRIC)
		{
			strIRType.Format(_T("Radiometric"));
			IR_Entry = ptrIRMode->GetEntryByName("Radiometric");
		}

		if (IsReadable(ptrIRMode))
		{

			ptrIRMode->SetIntValue(IR_Entry->GetValue());
		}

		logMessage.Format(strIRType + " Mode");
		AddLog(logMessage);
	}

	if (nIRFormat == Camera_IRFormat::RADIOMETRIC)
	{
		CFloatPtr lR = lDeviceParams->GetNode("R");
		CFloatPtr lB = lDeviceParams->GetNode("B");
		CFloatPtr lF = lDeviceParams->GetNode("F");
		CFloatPtr lO = lDeviceParams->GetNode("O");

		if (lR && lB && lF && lO)
		{
			double tmpR = lR->GetValue();
			double tmpB = lB->GetValue();
			double tmF = lF->GetValue();
			double tmO = lO->GetValue();
			m_tauPlanckConstants->R = static_cast<double>(tmpR);
			m_tauPlanckConstants->B = static_cast<double>(tmpB);
			m_tauPlanckConstants->F = static_cast<double>(tmF);
			m_tauPlanckConstants->O = static_cast<double>(tmO);
			m_bMeasCapable = true;
			bFlag = true;
		}
		else
			bFlag = false;

		// Gain (J1) and offset (J0)
		CIntegerPtr lJ0 = lDeviceParams->GetNode("J0");
		CFloatPtr lJ1 = lDeviceParams->GetNode("J1");

		if (lJ0 && lJ1)
		{
			int64_t tmp = 0;
			lJ0->GetValue(tmp);
			double tmJ1 = lJ1->GetValue();
			m_tauPlanckConstants->J0 = static_cast<ULONG>(tmp);
			m_tauPlanckConstants->J1 = static_cast<double>(tmJ1);
			m_bMeasCapable = true;
			bFlag = true;
		}
		else
			bFlag = false;
		// Spectral response
		CFloatPtr lX = lDeviceParams->GetNode("X");
		CFloatPtr la1 = lDeviceParams->GetNode("alpha1");
		CFloatPtr la2 = lDeviceParams->GetNode("alpha2");
		CFloatPtr lb1 = lDeviceParams->GetNode("beta1");
		CFloatPtr lb2 = lDeviceParams->GetNode("beta2");

		if (lX && la1 && la2 && lb1 && lb2)
		{

			double tmpX = lX->GetValue();
			double tma1 = la1->GetValue();
			double tma2 = la2->GetValue();
			double tmbeta1 = lb1->GetValue();
			double tmbeta2 = lb2->GetValue();

			m_spectralResponseParams->X = static_cast<double>(tmpX);
			m_spectralResponseParams->alpha1 = static_cast<double>(tma1);
			m_spectralResponseParams->alpha2 = static_cast<double>(tma2);
			m_spectralResponseParams->beta1 = static_cast<double>(tmbeta1);
			m_spectralResponseParams->beta2 = static_cast<double>(tmbeta2);
			m_bMeasCapable = true;
			bFlag = true;
		}
		else
			bFlag = false;

		if (m_bMeasCapable)
			Radiometric_Utility::doUpdateCalcConst(m_objectParams, m_spectralResponseParams, m_tauPlanckConstants);
	}

	logMessage.Format(_T("Set Streaming CameraParameters"), m_nHeight, m_nWidth);
	AddLog(logMessage);

	return bFlag;
}

// =============================================================================
CameraModelList CMDSSpinnakerSDKDlg::FindCameraModel(INodeMap* lDeviceParams)
{
	CString message;
	try 
	{
		// 모델명 파라미터 접근
		CStringPtr ptrModelName = lDeviceParams->GetNode("DeviceModelName");
		CString strModelName(ptrModelName->GetValue().c_str()); // Spinnaker의 CString을 MFC의 CString으로 변환
		CString sContentsLower = strModelName.MakeLower(); // 소문자 변환
		CameraModelList CamList = CAM_MODEL::None;
	
		if (sContentsLower.Find(_T("a50")) != -1)
			CamList = CAM_MODEL::A50;
		else if (sContentsLower.Find(_T("a70")) != -1)
			CamList = CAM_MODEL::A70;
		else if (sContentsLower.Find(_T("a320")) != -1)
			CamList = CAM_MODEL::A300;
		else if (sContentsLower.Find(_T("a615")) != -1)
			CamList = CAM_MODEL::A615;
		else if (sContentsLower.Find(_T("ax5")) != -1)
			CamList = CAM_MODEL::Ax5;
		else if (sContentsLower.Find(_T("ft1000")) != -1)
			CamList = CAM_MODEL::FT1000;
		else if (sContentsLower.Find(_T("pt1000")) != -1)
			CamList = CAM_MODEL::FT1000;
		else if (sContentsLower.Find(_T("xsc")) != -1)
			CamList = CAM_MODEL::XSC;
		else if (sContentsLower.Find(_T("blackfly")) != -1)
			CamList = CAM_MODEL::BlackFly;

		return CamList; // 확인된 카메라 모델 반환
	}
	catch (const Spinnaker::Exception& e)
	{
		std::cout << "Error FindCameraModel:" << e.what() << endl;
		message.Format(_T("Error FindCameraModel: %S"), e.what());
		AddLog(message);
		return CAM_MODEL::None; // 예외 발생 시 None 반환
	}
}

// =============================================================================
CString CMDSSpinnakerSDKDlg::DeterminePixelFormat(const CString& pixelFormatCode)
{
	// 픽셀 포맷 코드에 따라 문자열 결정
	if (pixelFormatCode == _T("Mono8"))
		return _T("Mono8");
	else if (pixelFormatCode == _T("Mono16"))
		return _T("Mono16");
	else if (pixelFormatCode == _T("Mono14"))
		return _T("Mono14");
	else if (pixelFormatCode == _T("YUV422_8_UYVY"))
		return _T("YUV422_8_UYVY");
	else if (pixelFormatCode == _T("RGB8PACK"))
		return _T("RGB8Pack");
	else
		return _T("Mono16");  // Default case, you can adjust it as necessary
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::CameraParamSetting(CameraPtr pCam)
{
	CString strLog;

	try 
	{
		INodeMap& lDeviceParams = pCam->GetNodeMap();
		m_CamModelList = FindCameraModel(&lDeviceParams);

		DWORD pixeltypeValue = ConvertHexValue(m_strPixelFormat);
		CEnumerationPtr ptrPixelFormat = lDeviceParams.GetNode("PixelFormat");
		CString strPixelFormat = DeterminePixelFormat(m_strPixelFormat);
		gcstring formatName(strPixelFormat.GetString());  // Convert CString to gcstring
		CEnumEntryPtr pixelFormatValue = ptrPixelFormat->GetEntryByName(formatName);

		if (pixelFormatValue.IsValid()) 
		{
			ptrPixelFormat->SetIntValue(pixeltypeValue);
		}

		strLog.Format(_T("Set PixelFormat to %s"), strPixelFormat);
		AddLog(strLog);

		return true;  // Indicate success
	}
	catch (const Spinnaker::Exception& e)
	{	
		strLog.Format(_T("Error setting parameters: %S"), e.what());
		AddLog(strLog);
		return false;  // Indicate failure
	}
}

// =============================================================================
int CMDSSpinnakerSDKDlg::UpdateHeightForCamera(int nHeight, int nWidth)
{
	if (nHeight <= 0 || nWidth <= 0)
	{
		return 0;
	}

	int heightAdjustment = 1392 / nWidth + (1392 % nWidth ? 1 : 0);

	if (0 < heightAdjustment)
	{
		nHeight += heightAdjustment;
	}

	return heightAdjustment;
}

bool CMDSSpinnakerSDKDlg::FFF_HeightSummary(CameraPtr pCam)
{
	CString logMessage;

	INodeMap& nodeMap = pCam->GetNodeMap();

	CIntegerPtr lHeight = nodeMap.GetNode("Height");
	CIntegerPtr lWidth = nodeMap.GetNode("Width");

	int A400_Height = 240;
	int A700_Height = 480;
	int A70_Height = 480;
	int A645_Height = 480;
	int A50_Height = 348;
	int Ax5_Height = 256;

	int nFFF_HeaderValue = UpdateHeightForCamera(lHeight->GetValue(), lWidth->GetValue());
	int nFinalHeight = 0;

	switch (m_CamModelList)
	{
	case CAM_MODEL::A300:
		lHeight->SetValue(246);
		//m_bTLUTCapable = true;
		break;

	case CAM_MODEL::A50:
		m_nHeight = A50_Height;
		nFinalHeight = nFFF_HeaderValue + A50_Height;
		lHeight->SetValue(nFinalHeight);
		break;

	case CAM_MODEL::A400:
		m_nHeight = A400_Height;
		nFinalHeight = nFFF_HeaderValue + A400_Height;
		lHeight->SetValue(nFinalHeight);
		break;

	case CAM_MODEL::A70:
	case CAM_MODEL::A615:
	case CAM_MODEL::A700:
	case CAM_MODEL::FT1000:
		m_nHeight = A645_Height;
		nFinalHeight = nFFF_HeaderValue + A645_Height;
		lHeight->SetValue(nFinalHeight);
		break;

	default:
		break;
	}
	m_nWidth = lWidth->GetValue();

	m_roi_rect.width = m_nWidth;
	m_roi_rect.height = m_nHeight;

	CString str;
	str.Format(_T("%d"), m_nWidth);
	SetDlgItemText(IDC_ED_ROI_WIDTH, str);
	str.Format(_T("%d"), m_nHeight);
	SetDlgItemText(IDC_ED_ROI_HEIGHT, str);

	logMessage.Format(_T("Set Data Height = [%d]"), nFinalHeight);
	AddLog(logMessage);

	logMessage.Format(_T("Display Height = [%d], Width = [%d]"), m_nHeight, m_nWidth);
	AddLog(logMessage);

	return true;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::AcquireImages(CameraPtr pCam)
{
	if (pCam == nullptr)
		return false;
	else
	{
		AddLog(_T("camera is not connected"));
		AddLog(_T("------------------------------------"));
	}

	bool bFlagCheck = false;
	CString logMessage;
	INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
	CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
	int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

	pCam->AcquisitionStop();

	FFF_HeightSummary(pCam);



	ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

	bFlagCheck = CameraParamSetting(pCam);

	FFF_HeightSummary(pCam);

	if (bFlagCheck)
	{
		// Begin acquiring images
		pCam->BeginAcquisition();
	}

	logMessage.Format(_T("Camera BeginAcquisition"));
	AddLog(logMessage);

	int rt = SetStreamingCameraParameters(&nodeMap);

	GetTempRangeSearch(pCam);
	GetIRFrameRates(pCam);

	PopulateComboBoxes_Params();

	AddLog(_T("------------------------------------"));
	return true;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (m_Device == nullptr)
	{
		AddLog(_T("------------------------------------"));
		m_Device = ConnectToDevice(m_nSelCamIndex);
	}

	if (m_Device != nullptr)
		StartProc();
}

void CMDSSpinnakerSDKDlg::CameraStart()
{
	AcquireImages(m_Device);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	DisConnectToDevice();
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnLbnSelchangeCamList()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nIndex = m_CamListBox.GetCurSel();  // 현재 선택된 인덱스 얻기
	int indexValue = 0;
	CString message;
	if (nIndex != LB_ERR)
	{
		CString strText;
		m_CamListBox.GetText(nIndex, strText);  // 선택된 인덱스의 텍스트 얻기

		// "Index = [" 문자열의 위치 찾기
		int startPos = strText.Find(_T("Camera Index: ["));
		if (startPos != -1)
		{
			startPos += 15; // "Index = ["의 길이는 9, 숫자가 시작하는 위치로 이동
			int endPos = strText.Find(_T("]"), startPos); // 시작 위치에서 ']'의 위치 찾기

			if (endPos != -1)
			{
				CString indexStr = strText.Mid(startPos, endPos - startPos);  // 인덱스 값 추출
				indexValue = _ttoi(indexStr);  // CString을 int로 변환
				m_nSelCamIndex = indexValue - 1;

				// 결과 출력 또는 사용
				message.Format(_T("Selected Extract Index: %d"), indexValue);
				AddLog(message);
			}
		}
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	deleteVariables();
	Camera_destroy();

	CDialogEx::OnClose();
}

// =============================================================================
LRESULT CMDSSpinnakerSDKDlg::OnAddLog(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(wParam);
	if (pStr != nullptr)
	{
		int nIndex = m_LogListBox.AddString(*pStr);  // 항목 추가하고 인덱스를 받아옴
		m_LogListBox.SetTopIndex(nIndex);  // 추가된 항목으로 스크롤 이동
		delete pStr;

		// 가로 스크롤바 설정 업데이트
		CreateHorizontalScroll();
	}
	m_LogListBox.Invalidate();
	m_LogListBox.UpdateWindow();

	return 0;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::CreateHorizontalScroll()
{
	CString str; CSize sz; int dx = 0;
	CDC* pDC = m_LogListBox.GetDC();

	for (int i = 0; i < m_LogListBox.GetCount(); i++)
	{
		m_LogListBox.GetText(i, str);
		sz = pDC->GetTextExtent(str);

		if (sz.cx > dx)
			dx = sz.cx;
	}
	m_LogListBox.ReleaseDC(pDC);

	if (m_LogListBox.GetHorizontalExtent() < dx)
	{
		m_LogListBox.SetHorizontalExtent(dx);
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::AddLog(LPCTSTR lpszFormat, ...)
{
	SYSTEMTIME cur_time;
	CString strDate;
	CString formattedMessage;
	va_list args;
	wchar_t  buffer[1024];  // 충분한 크기의 버퍼를 할당

	GetLocalTime(&cur_time);
	strDate.Format(_T("%02d:%02d:%02d"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond);

	va_start(args, lpszFormat);
	_vsnwprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t), _TRUNCATE, lpszFormat, args);
	va_end(args);

	formattedMessage = buffer;

	CString fullLogMessage;
	fullLogMessage.Format(_T("[%s] %s\r\n"), strDate, formattedMessage);  // 로그 라인에 개행 문자 추가

	// 로그 메시지 포스트
	const int maxRetries = 3;
	int retryCount = 0;
	bool messagePosted = false;

	CString* pStr = new CString(fullLogMessage);
	while (retryCount < maxRetries && !messagePosted)
	{
		if (PostMessage(WM_ADDLOG, reinterpret_cast<WPARAM>(pStr), 0))
		{
			messagePosted = true;
		}
		else {
			retryCount++;
			Sleep(100); // 100ms 대기 후 재시도
		}
	}
	if (!messagePosted)
	{
		// 메시지 전송 실패시 처리
		std::cerr << _T("Failed to post message to main window after ") << maxRetries << _T(" retries") << std::endl;
		delete pStr; // 메모리 누수를 방지하기 위해 pStr 삭제
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::StartThreadProc()
{
	// 초기화 확인 및 스레드 시작
	m_TStatus = ThreadStatus::THREAD_RUNNING;
	pThread = AfxBeginThread(ThreadCam, this, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_NEW_PROCESS_GROUP);
	if (pThread)
	{
		pThread->m_bAutoDelete = FALSE;
		pThread->ResumeThread();
	}

	// 소비자 쓰레드 시작
	pThreadConsumer = AfxBeginThread(ConsumerThreadProc, this, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_NEW_PROCESS_GROUP);
	if (pThreadConsumer)
	{
		pThreadConsumer->m_bAutoDelete = FALSE;
		pThreadConsumer->ResumeThread();
	}
}

// =============================================================================
UINT CMDSSpinnakerSDKDlg::ConsumerThreadProc(LPVOID param)
{
	CMDSSpinnakerSDKDlg* pDlg = static_cast<CMDSSpinnakerSDKDlg*>(param);
	pDlg->ConsumerThread();
	return 0;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::StartProc()
{
	if (m_Device == nullptr)
		return;

	SetPauseStreaming(true);

	if (m_TStatus == ThreadStatus::THREAD_IDLE)
	{
		CameraStart();
		StartThreadProc();
	}
	else
	{
		m_Device->AcquisitionStart();
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnSeq()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strLog;
	if (!m_bSaveAsSEQ)
	{
		m_SEQSave.SetWindowText(_T("Recording...."));
		SetStartRecordingFlag(true);

		strLog.Format(_T("---------Camera Video Start Recording"));
		AddLog(strLog);
	}
	else
	{
		m_SEQSave.SetWindowText(_T("SEQ Save"));
		StopRecording();

		strLog.Format(_T("---------Camera Video Stop Recording"));
		AddLog(strLog);
	}

}
// =============================================================================
UINT CMDSSpinnakerSDKDlg::ThreadCam(LPVOID _mothod)
{
	CString strLog = _T(""); // 로그 출력을 위한 문자열
	CameraPtr pCam = nullptr;

	CMDSSpinnakerSDKDlg* pDlg = static_cast<CMDSSpinnakerSDKDlg*>(_mothod);

	while (pDlg->m_TStatus == THREAD_STATUS::THREAD_RUNNING) // 쓰레드가 실행 중인 동안 반복
	{
		try
		{
			if (pDlg->GetPauseStreaming()) // 일시 정지 상태 검사
			{
				pCam = pDlg->m_Device;
				if (pCam == nullptr)
					continue; // 카메라 인스턴스 검사

				INodeMap& nodeMap = pCam->GetNodeMap();
				ImagePtr pBufferData = pCam->GetNextImage();
			
				std::lock_guard<std::mutex> lock(pDlg->queueMutex);

				pDlg->imageQueue.push(pBufferData);
				pDlg->queueCond.notify_one(); // 소비자 쓰레드에 알림
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
		catch (const Spinnaker::Exception& e)
		{

		}
	}
	return 0;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::ConsumerThread()
{
	auto lastProcessTime = std::chrono::high_resolution_clock::now();

	while (m_TStatus == THREAD_STATUS::THREAD_RUNNING)
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		queueCond.wait(lock, [this] { return !imageQueue.empty() || m_TStatus != THREAD_STATUS::THREAD_RUNNING; });

		while (!imageQueue.empty())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastProcessTime).count();
			ImagePtr pBufferData = nullptr;
			if (elapsed >= 100) // 마지막 처리로부터 1초 이상 경과했는지 체크
			{
				pBufferData = imageQueue.front();
				imageQueue.pop();
				lock.unlock(); // 잠금 해제

				if (pBufferData && !pBufferData->IsIncomplete() && pBufferData->GetImageStatus() == SPINNAKER_IMAGE_STATUS_NO_ERROR)
				{
					byte* pImageBuffer = static_cast<byte*>(pBufferData->GetData());

					if (GetStartRecordingFlag())
					{
						mm_timer.GetUTCTime(m_ts, m_ms, m_tzBias);
						AddBufferToQueue(pBufferData);
					}
					DataProcessing(pImageBuffer); // 데이터 처리
				}

				lastProcessTime = std::chrono::high_resolution_clock::now(); // 마지막 처리 시간 갱신
				std::this_thread::sleep_for(std::chrono::milliseconds(30)); // 처리 후 짧은 대기
				lock.lock(); // 잠금 재설정
			}
			else
			{
				// 시간 요건을 충족하지 못한 경우 이미지 버퍼 해제만 진행
				pBufferData = imageQueue.front();
				imageQueue.pop();
				pBufferData->Release();
			}
		}
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::DataProcessing(byte* buffer)
{
	if (buffer == nullptr)
		return;

	if (ImgProcThreadRunning && imageProcessingThread.joinable())
	{
		imageProcessingThread.join();  // 이미 실행 중인 스레드가 있다면 종료 기다림
	}
	ImgProcThreadRunning = true;
	imageProcessingThread = std::thread(&CMDSSpinnakerSDKDlg::RenderDataSequence, this, buffer);
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::IsRoiValid(const cv::Rect& roi, int imageWidth, int imageHeight)
{
	// ROI가 이미지 경계를 벗어나지 않는지 검사
	return roi.x >= 0 && roi.y >= 0 && (roi.x + roi.width) <= imageWidth && (roi.y + roi.height) <= imageHeight;
}

// =============================================================================
// 지정된 영역의 데이터 처리
std::unique_ptr<uint16_t[]> CMDSSpinnakerSDKDlg::extractROI(const uint8_t* byteArr, int nWidth, int nStartX, int nEndX, int nStartY, int nEndY, int roiWidth, int roiHeight)
{
	int nArraySize = roiWidth * roiHeight;
	std::unique_ptr<uint16_t[]> roiArray = std::make_unique<uint16_t[]>(nArraySize);

	int k = 0;
	for (int y = nStartY; y < nEndY; y++)
	{
		int rowOffset = y * nWidth;
		for (int x = nStartX; x < nEndX; x++)
		{
			int index = rowOffset + x;
			roiArray[k++] = static_cast<uint16_t>(byteArr[index * 2] + (byteArr[index * 2 + 1] << 8));
		}
	}
	return roiArray;
}

// =============================================================================
// 연산 변수 초기화
bool CMDSSpinnakerSDKDlg::InitializeTemperatureThresholds()
{
	// min, max 변수값 reset
	m_Max = 0;
	m_Min = 65535;
	m_Max_X = 0;
	m_Max_Y = 0;
	m_Min_X = 0;
	m_Min_Y = 0;

	return true;
}

// =============================================================================
// ROI 내부의 데이터 연산
void CMDSSpinnakerSDKDlg::ROIXYinBox(ushort uTempValue, double dScale, int nCurrentX, int nCurrentY, cv::Rect roi, int nPointIdx)
{
	int absoluteX = nCurrentX + roi.x;
	int absoluteY = nCurrentY + roi.y;
	CTemperature T;

	// 최소 온도 체크 및 업데이트
	if (uTempValue <= m_Min)
	{
		m_Min = uTempValue;
		m_Min_X = absoluteX;
		m_Min_Y = absoluteY;
		m_MinSpot.x = m_Min_X;  // 스팟의 X 좌표 업데이트
		m_MinSpot.y = m_Min_Y;  // 스팟의 Y 좌표 업데이트
		m_MinSpot.pointIdx = nPointIdx; // 포인트 인덱스 저장

		if (GetIRFormat() != Camera_IRFormat::RADIOMETRIC)
			m_MinSpot.tempValue = static_cast<float>(uTempValue) * dScale - FAHRENHEIT;
		else
		{
			T = m_RadUtill->imgToTemp(uTempValue, m_tauPlanckConstants);
			m_MinSpot.tempValue = T.Value(CTemperature::Celsius);
		}
	}
	// 최대 온도 체크 및 업데이트
	if (uTempValue >= m_Max)
	{
		m_Max = uTempValue;
		m_Max_X = absoluteX;
		m_Max_Y = absoluteY;
		m_MaxSpot.x = m_Max_X;  // 스팟의 X 좌표 업데이트
		m_MaxSpot.y = m_Max_Y;  // 스팟의 Y 좌표 업데이트
		m_MaxSpot.pointIdx = nPointIdx; // 포인트 인덱스 저장

		if (GetIRFormat() != Camera_IRFormat::RADIOMETRIC)
			m_MaxSpot.tempValue = static_cast<float>(uTempValue) * dScale - FAHRENHEIT;
		else
		{
			T = m_RadUtill->imgToTemp(uTempValue, m_tauPlanckConstants);
			m_MaxSpot.tempValue = T.Value(CTemperature::Celsius);
		}
	}

	m_Span = m_MaxSpot.tempValue - m_MinSpot.tempValue;
	m_Level = (m_MaxSpot.tempValue + m_MinSpot.tempValue) / 2.0;
}

// =============================================================================
// 16비트 데이터 최소,최대값 산출
void CMDSSpinnakerSDKDlg::ProcessImageData(std::unique_ptr<uint16_t[]>&& data, int size, int nImageWidth, int nImageHeight, cv::Rect roi)
{
	double dScale = GetScaleFactor();
	InitializeTemperatureThresholds();

	int nPointIdx = 0; // 포인트 인덱스 초기화

	for (int y = 0; y < roi.height; y++)
	{
		for (int x = 0; x < roi.width; x++)
		{
			int dataIndex = y * roi.width + x; // ROI 내에서의 인덱스 계산
			ushort tempValue = static_cast<ushort>(data[dataIndex]);

			ROIXYinBox(tempValue, dScale, x, y, roi, nPointIdx);

			nPointIdx++; // 포인트 인덱스 증가
		}
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::RenderDataSequence(byte* imageDataPtr)
{
	int nWidth = m_nWidth;
	int nHeight = m_nHeight;

	int nArraysize = nWidth * nHeight;

	std::unique_ptr<uint16_t[]> fulldata = nullptr;
	std::unique_ptr<uint16_t[]> roiData = nullptr;

	fulldata = extractWholeImage(imageDataPtr, nArraysize, nWidth, nHeight);

	UpdateTriggerCountsAndLineStates(std::move(fulldata), nWidth, nHeight);

	cv::Rect roi = m_roi_rect;
	if (IsRoiValid(roi, nWidth, nHeight))
	{
		roiData = extractROI(imageDataPtr, nWidth, roi.x, roi.x + roi.width, roi.y, roi.y + roi.height, roi.width, roi.height);
		ProcessImageData(std::move(roiData), roi.width * roi.height, nWidth, nHeight, roi);
	}

	// 이미지 노멀라이즈
	cv::Mat processedImage = NormalizeAndProcessImage(imageDataPtr, nHeight, nWidth, CV_16UC1);
	cv::Mat displayImage = DisplayLiveImage(processedImage, false);

	if (!displayImage.empty())
	{
		m_CurrentImageMat = cv::Mat::zeros(nHeight, nWidth, CV_16UC1);
		displayImage.copyTo(m_CurrentImageMat);
	}
	try 
	{
		cv::Mat* pDisplayMat = new cv::Mat(displayImage);
		if (!AfxGetApp()->GetMainWnd()->PostMessage(WM_UPDATE_IMAGE, reinterpret_cast<WPARAM>(pDisplayMat), 0))
		{
			//delete pDisplayMat; // 메시지 전송 실패 시 메모리 해제
		}

		if (GetMouseImageSaveFlag())
		{
			SaveImage(displayImage);
			SetMouseImageSaveFlag(FALSE);
		}

		if (GetStartRecordingFlag())
		{
			if (StartRecording(nWidth, nHeight, CameraFPS(m_Device)))
			{
				ClearQueue();
				m_bSaveAsSEQ = true;
			}

		}

		CleanupAfterProcessing();
	}
	catch (const std::exception& e) 
	{
		// 오류 로깅 등 예외 처리
		AddLog(CString("Exception in RenderDataSequence: %S") + e.what());
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(0));
}

// =============================================================================
void CMDSSpinnakerSDKDlg::UpdateTriggerCountsAndLineStates(std::unique_ptr<uint16_t[]>&& data, int nWidth, int nHeight)
{
	FPGA_HEADER* pFPGA;

	if (GetStartRecordingFlag())
	{
		if (m_CamModelList != CAM_MODEL::Ax5 || m_CamModelList != CAM_MODEL::BlackFly)
		{
			// Check trig count
			pFPGA = (FPGA_HEADER*)&data[nHeight * nWidth];
			if (pFPGA->dp1_trig_type & FPGA_TRIG_TYPE_MARK)
			{
				m_TrigCount1++;
				m_TrigCount++;
			}

			m_LineState1 = pFPGA->dp1_trig_state ? 1 : 0;
			m_LineState2 = pFPGA->dp2_trig_state ? 1 : 0;

			if (pFPGA->dp2_trig_type & FPGA_TRIG_TYPE_MARK)
			{
				m_TrigCount2++;
				m_TrigCount++;
			}
		}
	}
}

// =============================================================================
cv::Mat CMDSSpinnakerSDKDlg::DisplayLiveImage(cv::Mat& processedImageMat, bool bGrayType)
{
	if (processedImageMat.empty())
		return cv::Mat();

	cv::Mat GrayImage, ResultImage;

	// Gray Scale
	if (bGrayType)
	{
		cv::cvtColor(processedImageMat, GrayImage, cv::COLOR_GRAY2BGR);
		cv::normalize(GrayImage, ResultImage, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::cvtColor(ResultImage, ResultImage, cv::COLOR_BGR2GRAY);
	}
	//color Scale
	else
	{
		ResultImage = MapColorsToPalette(processedImageMat, GetPaletteType());
		if (ResultImage.type() == CV_8UC1)
		{
			cv::cvtColor(ResultImage, ResultImage, cv::COLOR_GRAY2BGR);
		}
	}
	
	DrawRoiRectangle(ResultImage);

	CreateAndDrawBitmap(ResultImage);

	return ResultImage;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::DrawRoiRectangle(cv::Mat& image)
{
	cv::Scalar MinColor(255, 0, 0); // 흰색 (BGR 순서로 지정)
	cv::Scalar MaxColor(0, 0, 255); // 흰색 (BGR 순서로 지정)
	cv::Scalar drawColor(51, 255, 51);

	int fontFace = cv::FONT_HERSHEY_SIMPLEX;
	double fontScale = 0.5;

	int markerSize = 15; // 마커의 크기 설정

	cv::Rect roi = m_roi_rect;
	cv::rectangle(image, roi, drawColor, 1);
	
	// ROI 인덱스를 텍스트로 변환
	std::string text = "[ROI]";
	cv::Point textOrg(roi.x, roi.y - 5); // 도형의 좌상단에 텍스트 위치

	// ROI 인덱스를 좌상단에 표시
	cv::putText(image, text, textOrg, fontFace, fontScale, drawColor, 1);

	cv::Scalar TextColor(255, 255, 255); // 흰색 (BGR 순서로 지정)
	cv::Point Mincenter(m_MinSpot.x, m_MinSpot.y);
	cv::Point MincenterText(m_MinSpot.x, m_MinSpot.y);
	std::string Mintext =  "[" + std::to_string(static_cast<int>(m_MinSpot.tempValue)) + "]";
	if (Mincenter.x >= 0 && Mincenter.x < image.cols && Mincenter.y >= 0 && Mincenter.y < image.rows)
	{
		// 텍스트 그리기
		cv::putText(image, Mintext, MincenterText, cv::FONT_HERSHEY_PLAIN, 1, TextColor, 1, cv::LINE_AA);
		// 마커 그리기
		cv::drawMarker(image, Mincenter, MinColor, cv::MARKER_TRIANGLE_DOWN, markerSize, 2);
	}

	cv::Point Maxcenter(m_MaxSpot.x, m_MaxSpot.y);
	std::string Maxtext = "[" + std::to_string(static_cast<int>(m_MaxSpot.tempValue)) + "]";
	if (Maxcenter.x >= 0 && Maxcenter.x < image.cols && Maxcenter.y >= 0 && Maxcenter.y < image.rows)
	{
		// 텍스트 그리기
		cv::putText(image, Maxtext, Maxcenter, cv::FONT_HERSHEY_PLAIN, 1, TextColor, 1, cv::LINE_AA);
		// 마커 그리기
		cv::drawMarker(image, Maxcenter, MaxColor, cv::MARKER_TRIANGLE_UP, markerSize, 2);
	}
}

// =============================================================================
//이미지 데이터에 따라 파레트 설정
cv::Mat CMDSSpinnakerSDKDlg::MapColorsToPalette(const cv::Mat& inputImage, PaletteTypes palette)
{
	// Input 이미지는 CV_16UC1 이다
	cv::Mat normalizedImage(inputImage.size(), CV_16UC1); // 정규화된 16비트 이미지

	// input 이미지 내에서 최소값과 최대값 검색.
	double minVal, maxVal;
	cv::minMaxLoc(inputImage, &minVal, &maxVal);

	// [0, 65535] 범위로 정규화
	inputImage.convertTo(normalizedImage, CV_16UC1, 65535.0 / (maxVal - minVal), -65535.0 * minVal / (maxVal - minVal));

	// 컬러 맵 적용은 일반적으로 8비트 이미지에 사용되므로,
	// 여기에서는 정규화된 이미지를 임시로 8비트로 변환하여 컬러 맵을 적용.
	cv::Mat normalizedImage8bit;
	normalizedImage.convertTo(normalizedImage8bit, CV_8UC1, 255.0 / 65535.0);

	cv::Mat normalizedImage3channel(inputImage.size(), CV_8UC3);
	cv::cvtColor(normalizedImage8bit, normalizedImage3channel, cv::COLOR_GRAY2BGR);
	// 메모리 해제
	normalizedImage8bit.release();

	cv::Mat colorMappedImage;

	// 아이언 1/2/1
	colorMappedImage = applyIronColorMap(normalizedImage3channel, palette);

	// 메모리 해제
	normalizedImage3channel.release();

	return colorMappedImage;
}

// =============================================================================
cv::Mat CMDSSpinnakerSDKDlg::applyIronColorMap(cv::Mat& im_gray, PaletteTypes palette)
{
	// 팔레트 가져오기
	std::vector<cv::Vec3b> Selectedpalette = paletteManager.GetPalette(palette);

	// 스케일을 적용한 조정된 팔레트 생성
	std::vector<cv::Vec3b> adjusted_palette = adjustPaletteScale(Selectedpalette);

	//// 그레이스케일 이미지의 최소 및 최대 값 찾기
	double minVal, maxVal;
	cv::minMaxLoc(im_gray, &minVal, &maxVal);

	// 그레이스케일 이미지를 0-255 범위로 재조정
	cv::Mat im_gray_rescaled;
	im_gray.convertTo(im_gray_rescaled, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));

	// LUT 생성
	cv::Mat lut = CreateLUT(adjusted_palette);
	std::cout << "LUT - Type: " << lut.type() << ", Size: " << lut.size() << std::endl;

	if (lut.empty() || lut.type() != CV_8UC3 || lut.rows != 256 || lut.cols != 1) 
	{
		std::cerr << "Error: LUT must be 256x1 3-channel matrix." << std::endl;
		return cv::Mat();  // LUT가 올바르지 않음
	}

	// LUT를 사용하여 그레이스케일 이미지를 컬러 이미지로 변환
	cv::Mat im_color;
	try
	{
		im_color.create(im_gray_rescaled.size(), CV_8UC3);  // 출력 이미지 초기화
		cv::LUT(im_gray_rescaled, lut, im_color);
	}
	catch (const cv::Exception& e) 
	{
		CString strLog;
		strLog.Format(_T("Error during cv::LUT: %S"), e.what());
		AddLog(strLog);
		return cv::Mat();  // 오류 발생 시 빈 이미지 반환
	}

	return im_color;
}

// =============================================================================
std::vector<cv::Vec3b> CMDSSpinnakerSDKDlg::adjustPaletteScale(const std::vector<cv::Vec3b>& originalPalette)
{
	std::vector<cv::Vec3b> adjustedPalette;
	for (const cv::Vec3b& color : originalPalette)
	{
		// 각 색상의 B, G, R 값을 스케일링
		uchar newB = static_cast<uchar>(color[2]);
		uchar newG = static_cast<uchar>(color[1]);
		uchar newR = static_cast<uchar>(color[0]);

		// 값이 255를 초과하지 않도록 제한
		newB = std::min(255, (int)newB);
		newG = std::min(255, (int)newG);
		newR = std::min(255, (int)newR);

		adjustedPalette.push_back(cv::Vec3b(newB, newG, newR));
	}
	return adjustedPalette;
}

// =============================================================================
cv::Mat CMDSSpinnakerSDKDlg::CreateLUT(const std::vector<cv::Vec3b>& adjusted_palette)
{
	cv::Mat b(256, 1, CV_8U), g(256, 1, CV_8U), r(256, 1, CV_8U);
	for (int i = 0; i < 256; ++i) 
	{
		b.at<uchar>(i) = adjusted_palette[i][0];
		g.at<uchar>(i) = adjusted_palette[i][1];
		r.at<uchar>(i) = adjusted_palette[i][2];
	}
	cv::Mat channels[] = { b, g, r };
	cv::Mat lut;
	cv::merge(channels, 3, lut);
	return lut;
}

// =============================================================================
// 컬러맵 정보를 맴버변수에 저장
void CMDSSpinnakerSDKDlg::SetPaletteType(PaletteTypes type)
{
	m_colormapType = type;
}

// =============================================================================
PaletteTypes CMDSSpinnakerSDKDlg::GetPaletteType()
{
	return m_colormapType;
}

// =============================================================================
//화면에 출력한 비트맵 데이터를 생성
void CMDSSpinnakerSDKDlg::CreateAndDrawBitmap(const cv::Mat& imageMat)
{
	if (imageMat.empty()) return;

	m_BitmapInfo.reset(CreateBitmapInfo(imageMat)); // 스마트 포인터로 메모리 관리

	DrawImage(imageMat, IDC_CAM, m_BitmapInfo.get());
}

// 메시지 처리 핸들러
// =============================================================================
LRESULT CMDSSpinnakerSDKDlg::OnDisplayImage(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<cv::Mat> pImage(reinterpret_cast<cv::Mat*>(wParam));
	if (pImage)
	{
		CreateAndDrawBitmap(*pImage);
	}
	return 0;
}

// =============================================================================
// description : Mat data to BITMAPINFO
// 라이브 이미지 깜박임을 최소화하기 위하여 더블 버퍼링 기법 사용
void CMDSSpinnakerSDKDlg::DrawImage(Mat mImage, int nID, BITMAPINFO* BitmapInfo)
{
	// 입력 이미지 또는 BITMAPINFO가 유효하지 않은 경우 함수를 종료합니다.
	if (mImage.empty() || BitmapInfo == nullptr) 
	{
		//CString errorLog;
		//errorLog.Format(_T("Invalid input data. Image empty: %d, BitmapInfo null: %d"), mImage.empty(), BitmapInfo == nullptr);
		//AddLog(errorLog);
		return;
	}

	CClientDC dc(GetDlgItem(nID));
	if (!dc) 
	{
		AddLog(_T("Failed to get device context."));
		return;
	}

	CRect rect;
	GetDlgItem(nID)->GetClientRect(&rect);

	// 더블 버퍼링을 위한 준비
	CDC backBufferDC;
	CBitmap backBufferBitmap;
	if (!backBufferDC.CreateCompatibleDC(&dc))
	{
		AddLog(_T("Failed to create compatible DC."));
		return;
	}
	if (!backBufferBitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height())) 
	{
		backBufferDC.DeleteDC();
		AddLog(_T("Failed to create compatible bitmap."));
		return;
	}

	CBitmap* pOldBackBufferBitmap = backBufferDC.SelectObject(&backBufferBitmap);
	if (!pOldBackBufferBitmap)
	{
		backBufferBitmap.DeleteObject();
		backBufferDC.DeleteDC();
		AddLog(_T("Failed to select bitmap object into DC."));
		return;
	}

	// 이미지 스트레칭
	backBufferDC.SetStretchBltMode(COLORONCOLOR);
	if (GDI_ERROR == StretchDIBits(backBufferDC.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, mImage.cols, mImage.rows, mImage.data, BitmapInfo, DIB_RGB_COLORS, SRCCOPY))
	{
		AddLog(_T("StretchDIBits failed."));
	}

	// 더블 버퍼링을 사용하여 화면에 그리기
	if (!dc.BitBlt(0, 0, rect.Width(), rect.Height(), &backBufferDC, 0, 0, SRCCOPY)) 
	{
		AddLog(_T("BitBlt failed."));
	}

	// 정리
	backBufferDC.SelectObject(pOldBackBufferBitmap);
	backBufferBitmap.DeleteObject();
	backBufferDC.DeleteDC();
}

// =============================================================================
// BITMAPINFO, BITMAPINFOHEADER  setting
BITMAPINFO* CMDSSpinnakerSDKDlg::CreateBitmapInfo(const cv::Mat& imageMat)
{
	BITMAPINFO* bitmapInfo = nullptr;
	size_t bitmapInfoSize = 0;
	int imageSize = 0;
	int bpp = 0;
	int nbiClrUsed = 0;
	int nType = imageMat.type();
	int w = imageMat.cols;
	int h = imageMat.rows;
	try
	{
		bpp = (int)imageMat.elemSize() * 8;

		bitmapInfoSize = sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
		bitmapInfo = reinterpret_cast<BITMAPINFO*>(new BYTE[bitmapInfoSize]);
		imageSize = w * h;
		nbiClrUsed = 256;
		memcpy(bitmapInfo->bmiColors, GrayPalette, 256 * sizeof(RGBQUAD));

		BITMAPINFOHEADER& bmiHeader = bitmapInfo->bmiHeader;

		bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiHeader.biWidth = w;
		bmiHeader.biHeight = -h;  // 이미지를 아래에서 위로 표시
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = bpp;
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = imageSize; // 이미지 크기 초기화
		bmiHeader.biClrUsed = nbiClrUsed;   // 색상 팔레트 적용

		// 초기화 상태 출력

		return bitmapInfo;
	}
	catch (const std::bad_alloc&)
	{
		delete[] reinterpret_cast<BYTE*>(bitmapInfo); // 메모리 해제
		throw std::runtime_error("Failed to allocate memory for BITMAPINFO");
	}
}

// =============================================================================
// 이미지 처리 중 동적 생성 객체 메모리 해제
void CMDSSpinnakerSDKDlg::CleanupAfterProcessing()
{

	//m_BitmapInfo.reset(); 
	//if (m_BitmapInfo != nullptr)
	//{
	//	delete[] reinterpret_cast<BYTE*>(m_BitmapInfo); // 메모리 해제
	//	m_BitmapInfo = nullptr;
	//}
}

// =============================================================================
template <typename T>
cv::Mat CMDSSpinnakerSDKDlg::NormalizeAndProcessImage(const T* data, int height, int width, int cvType)
{
	if (data == nullptr)
	{
		// data가 nullptr일 경우 예외처리 또는 기본값 설정을 수행할 수 있습니다.
		// 여기서는 기본값으로 빈 이미지를 반환합니다.
		return cv::Mat();
	}

	// Mono16 고정
	cv::Mat imageMat(height, width, cvType, (void*)data);
	constexpr int nTemp = std::numeric_limits<T>::max();
	cv::normalize(imageMat, imageMat, 0, std::numeric_limits<T>::max(), cv::NORM_MINMAX);
	double min_val = 0, max_val = 0;
	cv::minMaxLoc(imageMat, &min_val, &max_val);

	int range = static_cast<int>(max_val) - static_cast<int>(min_val);

	if (range == 0)
		range = 1;

	return imageMat;
}

// =============================================================================
std::unique_ptr<uint16_t[]> CMDSSpinnakerSDKDlg::extractWholeImage(const uint8_t* byteArr, int byteArrLength, int nWidth, int nHeight)
{
	// 전체 영역의 크기 계산
	int imageSize = nWidth * nHeight;

	// imageSize 크기의 uint16_t 배열을 동적으로 할당합니다.
	std::unique_ptr<uint16_t[]> imageArray = std::make_unique<uint16_t[]>(imageSize);
	for (int i = 0; i < imageSize; ++i)
	{
		uint16_t nRawdata = static_cast<uint16_t>(byteArr[i * 2] | (byteArr[i * 2 + 1] << 8));
		if (GetIRFormat() != Camera_IRFormat::RADIOMETRIC)
			imageArray[i] = (nRawdata * GetScaleFactor()) - FAHRENHEIT;
		else
			imageArray[i] = nRawdata;
	}

	return imageArray; // std::unique_ptr 반환
}

// =============================================================================
// string to hex
DWORD CMDSSpinnakerSDKDlg::ConvertHexValue(const CString& hexString)
{
	DWORD hexValue = 0;
	CString strWithoutPrefix = hexString;

	// CString에서 '0x' 또는 '0X' 접두사를 제거
	if (strWithoutPrefix.Left(2).CompareNoCase(_T("0x")) == 0)
		strWithoutPrefix = strWithoutPrefix.Mid(2);

	// 16진수 문자열을 숫자로 변환
	_stscanf_s(strWithoutPrefix, _T("%X"), &hexValue);

	return hexValue;
}

// =============================================================================
LRESULT CMDSSpinnakerSDKDlg::OnUpdateImage(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<cv::Mat> pImage(reinterpret_cast<cv::Mat*>(wParam));
	if (pImage)
	{
		CreateAndDrawBitmap(*pImage);
	}
	return 0;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::ApplyColorSettings(PaletteTypes selectedMap)
{
	if (m_Device != nullptr)
	{
		SetPaletteType(selectedMap);
	}
}

// =============================================================================
// 컬러맵 인덱스를 반환한다
PaletteTypes CMDSSpinnakerSDKDlg::GetSelectedColormap(CComboBox& comboControl)
{
	int selectedIndex = comboControl.GetCurSel(); // 현재 선택된 아이템의 인덱스 선택
	if (selectedIndex != CB_ERR) // 유효한 선택인지 확인.
	{
		return static_cast<PaletteTypes>(selectedIndex); // 인덱스를 ColormapTypes 열거형으로 캐스팅합니다.
	}
	else
	{
		// 오류 처리. 예를 들어, 기본 값을 반환할 수 있습니다.
		return PALETTE_TYPE::PALETTE_IRON;
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnCbnSelchangeCbColormap()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CB_COLORMAP);
	if (pComboBox)
	{
		PaletteTypes selectedMap = GetSelectedColormap(*pComboBox);
		ApplyColorSettings(selectedMap);
		pComboBox->RedrawWindow();
		pComboBox->Invalidate();
		pComboBox->UpdateData();
	}
}

// =============================================================================
// 컬러맵 콤보박스 초기값 설정
void CMDSSpinnakerSDKDlg::PopulateComboBoxes()
{
	// 할당된 콤보박스 아이템 데이터를 모두 동일하게 초기값 설정
	CComboBox* comboBoxes = &m_cbColormap;
	CComboBox* comboBoxes_IR = &m_cbIRFormat;
	int arrayLength = 0, arrayLength_IR = 0;

	CComboBox* combo = comboBoxes;
	combo->ResetContent();

	combo->ModifyStyle(0, CBS_DROPDOWN);

	for (int i = 0; ColormapArray::colormapStrings[i] != nullptr; ++i)
	{
		combo->AddString(ColormapArray::colormapStrings[i]);
		combo->SetItemData(i, LIGHTGREEN);

	}
	// 초기 선택값을 설정 (예: 첫 번째 항목을 선택)
	combo->SetCurSel(0); // 초기값 Iron color 
	combo->Invalidate();
	combo->UpdateWindow();
	
	while (MDS_IRFormatArray::MDS_IRFormatStrings[arrayLength_IR] != nullptr)
	{
		++arrayLength_IR;
	}

	// IR Format String Add
	comboBoxes_IR->ModifyStyle(0, CBS_DROPDOWN);
	for (int i = 0; MDS_IRFormatArray::MDS_IRFormatStrings[i] != nullptr; ++i)
	{
		comboBoxes_IR->AddString(MDS_IRFormatArray::MDS_IRFormatStrings[i]);
		comboBoxes_IR->SetItemData(i, LIGHTGREEN);
	}
	comboBoxes_IR->SetCurSel(0); // 초기값 RADIOMETRIC 
	comboBoxes_IR->Invalidate();
	comboBoxes_IR->UpdateWindow();
}

// =============================================================================
// 컬러맵 콤보박스 초기값 설정
void CMDSSpinnakerSDKDlg::PopulateComboBoxes_Params()
{
	CComboBox* comboBoxes_Range = &m_cbTempRange;

	if (m_cbTempRange.GetCount() > 1)
		m_cbTempRange.ResetContent();

	CString strRange;
	int nQCnt = m_nQueryCnt;

	// Temp Range String Add
	comboBoxes_Range->ModifyStyle(0, CBS_DROPDOWN);
	for (int j = 0; j < nQCnt; ++j)
	{
		strRange.Format(_T("%.2f ~ %.2f"), m_dQueryCaseLowLimit[j] - FAHRENHEIT, m_dQueryCaseHighLimit[j] - FAHRENHEIT);
		comboBoxes_Range->AddString(strRange);
		//comboBoxes_Range->SetItemData(j, LIGHTGREEN);
	}
	comboBoxes_Range->SetCurSel(0); // 초기값 RADIOMETRIC 
	comboBoxes_Range->Invalidate();
	comboBoxes_Range->UpdateData();

	CComboBox* comboBoxes_FPS = &m_cbFPS;

	if (m_cbFPS.GetCount() > 1)
		m_cbFPS.ResetContent();

	CString strFPS;
	int nIRsize = m_IRFrameRates.size();

	// Temp Range String Add
	comboBoxes_FPS->ModifyStyle(0, CBS_DROPDOWN);
	for (int j = 0; j < nIRsize; ++j)
	{
		comboBoxes_FPS->AddString(m_IRFrameRateNames[j]);
		//comboBoxes_FPS->SetItemData(j, LIGHTGREEN);
	}
	comboBoxes_FPS->SetCurSel(0); // 초기값 RADIOMETRIC 
	comboBoxes_FPS->Invalidate();
	comboBoxes_FPS->UpdateData();
}

//=============================================================================
int CMDSSpinnakerSDKDlg::FocusPos(CameraPtr pCam)
{
	CIntegerPtr ptrPos = pCam->GetNodeMap().GetNode("FocusPos");
	if (IsReadable(ptrPos))
	{
		return ptrPos->GetValue();
	}
}
//=============================================================================
int CMDSSpinnakerSDKDlg::FocusMethod(CameraPtr pCam, int methodIndex)
{
	try
	{
		// 자동 초점 메소드 노드 접근
		CEnumerationPtr ptrAutoFocusMethod = pCam->GetNodeMap().GetNode("AutoFocusMethod");
		if (!IsAvailable(ptrAutoFocusMethod) || !IsWritable(ptrAutoFocusMethod))
		{
			CString strLog;
			strLog.Format(_T("AutoFocusMethod node is not available or not writable."));
			AddLog(strLog);
			return -1; // 노드 접근 불가능하거나 쓰기 불가능
		}

		// 자동 초점 메소드의 가능한 엔트리 가져오기
		NodeList_t entries;
		ptrAutoFocusMethod->GetEntries(entries);

		if (methodIndex >= entries.size()) {
			CString strLog;
			strLog.Format(_T("Invalid index for AutoFocusMethod: %d"), methodIndex);
			AddLog(strLog);
			return -1; // 잘못된 인덱스 입력
		}

		CEnumEntryPtr pEntry = CEnumEntryPtr(entries[methodIndex]);
		if (!IsAvailable(pEntry) || !IsReadable(pEntry))
		{
			CString strLog;
			strLog.Format(_T("AutoFocusMethod entry is not available or readable: %S"), pEntry->GetSymbolic().c_str());
			AddLog(strLog);
			return -1; // 엔트리 접근 불가능
		}

		// 자동 초점 메소드 설정
		ptrAutoFocusMethod->SetIntValue(pEntry->GetValue());

		CString strLog;
		strLog.Format(_T("AutoFocus method set to %S successfully."), pEntry->GetSymbolic().c_str());
		AddLog(strLog);
		return 0; // 성공
	}
	catch (const Spinnaker::Exception& e)
	{
		CString strLog;
		strLog.Format(_T("Error setting AutoFocus method: %S"), e.what());
		AddLog(strLog);
		return -1; // 예외 발생 시 오류 반환
	}
}

//=============================================================================
double CMDSSpinnakerSDKDlg::CameraFPS(CameraPtr pCam)
{
	bool bFlag = false;

	CFloatPtr ptrFPS = pCam->GetNodeMap().GetNode("AcquisitionFrameRate");
	if (IsReadable(ptrFPS))
	{
		return ptrFPS->GetValue();
	}
	else
	{
		return GetSelectedComboBoxNumber(IDC_CB_FPS);
	} 
}

//=============================================================================
bool CMDSSpinnakerSDKDlg::SetAutoFocus()
{
	if (m_Device == nullptr)
		return false;

	bool bFlag = false;
	int nFocusPos = 0;
	CCommandPtr AutoFocus = m_Device->GetNodeMap().GetNode("AutoFocus");

	if (AutoFocus)
		AutoFocus->Execute();

	nFocusPos = FocusPos(m_Device);
	CString strlog;
	strlog.Format(_T("AutoFocus Success Focus Pos = [%d]"), nFocusPos);
	AddLog(strlog);

	bFlag = true;
	return bFlag;
}

//=============================================================================
bool CMDSSpinnakerSDKDlg::SetNUC()
{
	if (m_Device == nullptr)
		return false;

	bool bFlag = false;
	int nFocusPos = 0;
	CCommandPtr NUCAction = m_Device->GetNodeMap().GetNode("NUCAction");

	if (NUCAction)
		NUCAction->Execute();

	CString strlog;
	strlog.Format(_T("NUCAction Command Success"));
	AddLog(strlog);

	bFlag = true;
	return bFlag;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::GetTempRangeSearch(CameraPtr pCam)
{
	// 실화상 카메라는 온도범위 레인지가 필요없음
	if (m_CamModelList == CAM_MODEL::BlackFly)
		return false;

	// 카메라의 노드 맵을 가져옵니다.
	INodeMap& nodeMap = pCam->GetNodeMap();
	int nCurrentCaseCnt = 0;
	int64_t nPreviousCaseValue = -1; // 이전 쿼리 케이스 값을 저장하기 위한 변수
	CString strLog;
	bool bRtn = false;

	for (int i = 0; i < QUERYCOUNT; i++)
	{
		try
		{
			// QueryCase 값을 설정합니다.
			CIntegerPtr ptrQueryCase = nodeMap.GetNode("QueryCase");
			if (IsWritable(ptrQueryCase))
			{
				ptrQueryCase->SetValue(i);
			}

			// QueryCaseEnabled 값을 확인합니다.
			CBooleanPtr ptrQueryCaseEnabled = nodeMap.GetNode("QueryCaseEnabled");
			if (IsReadable(ptrQueryCaseEnabled) && ptrQueryCaseEnabled->GetValue())
			{
				int64_t nCurrentCaseValue = ptrQueryCase->GetValue();

				// 이전 쿼리 케이스 값과 현재 값이 다를 경우에만 nCurrentCaseCnt를 증가시킴
				if (nCurrentCaseValue != nPreviousCaseValue)
				{
			
					m_nCurrentCase[nCurrentCaseCnt] = nCurrentCaseValue;

					// QueryCaseLowLimit 값을 가져옵니다.
					CFloatPtr ptrLowLimit = nodeMap.GetNode("QueryCaseLowLimit");
					if (IsReadable(ptrLowLimit))
					{
						m_dQueryCaseLowLimit[nCurrentCaseCnt] = ptrLowLimit->GetValue();
					}

					// QueryCaseHighLimit 값을 가져옵니다.
					CFloatPtr ptrHighLimit = nodeMap.GetNode("QueryCaseHighLimit");
					if (IsReadable(ptrHighLimit))
					{
						m_dQueryCaseHighLimit[nCurrentCaseCnt] = ptrHighLimit->GetValue();
					}

					// 로그를 기록합니다.
					strLog.Format(_T("QueryCase Value =  [%d] QueryCase LowLimit = %.2f QueryCase HighLimit = %.2f"), nCurrentCaseValue,
						m_dQueryCaseLowLimit[nCurrentCaseCnt], m_dQueryCaseHighLimit[nCurrentCaseCnt]);
					AddLog(strLog);

					nCurrentCaseCnt++;

					// 현재 값을 이전 쿼리 케이스 값으로 설정
					nPreviousCaseValue = nCurrentCaseValue;
					
				}
			}
		}
		catch (Spinnaker::Exception& e)
		{
			// 예외 처리
			CString errorLog;
			errorLog.Format(_T("Error occurred: %S"), e.what());
			AddLog(errorLog);
			bRtn = false;
			return bRtn;
		}
	}
	
	m_nQueryCnt = nCurrentCaseCnt;
	bRtn = true;

	return bRtn;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::GetIRFrameRates(CameraPtr pCam)
{
	// 카메라의 노드 맵을 가져옵니다.
	INodeMap& nodeMap = pCam->GetNodeMap();
	CString strLog;
	bool bRtn = false;

	// 프레임 레이트 값을 저장할 벡터 초기화
	m_IRFrameRates.clear();
	m_IRFrameRateNames.clear();

	try
	{
		// IRFrameRate 노드를 가져옵니다.
		CEnumerationPtr ptrIRFrameRate = nodeMap.GetNode("IRFrameRate");
		if (IsReadable(ptrIRFrameRate))
		{
			// 가능한 프레임 레이트 값들을 열거
			NodeList_t entries;
			ptrIRFrameRate->GetEntries(entries);

			for (size_t i = 0; i < entries.size(); ++i)
			{
				CEnumEntryPtr ptrEntry = entries.at(i);
				if (IsReadable(ptrEntry))
				{
					int64_t value = ptrEntry->GetValue();
					GenICam::gcstring entryName = ptrEntry->GetSymbolic();

					m_IRFrameRates.push_back(value);
					m_IRFrameRateNames.push_back(CString(entryName.c_str()));

					// 로그를 기록합니다.
	/*				strLog.Format(_T("IRFrameRate Name = [%s], Value = [%lld]"), CString(entryName.c_str()), value);
					AddLog(strLog);*/
				}
			}

			bRtn = !m_IRFrameRates.empty();
		}
		else
		{
			strLog.Format(_T("Unable to read IRFrameRate."));
			AddLog(strLog);
		}
	}
	catch (Spinnaker::Exception& e)
	{
		// 예외 처리
		strLog.Format(_T("Error occurred: %S"), e.what());
		AddLog(strLog);
		return false;
	}

	return bRtn;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnCbnSelchangeCbFps()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_Device == nullptr)
		return;

	CString strLog;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CB_FPS);
	if (pComboBox)
	{
		int nSel = pComboBox->GetCurSel();
		if (nSel != CB_ERR)
		{
			int64_t selectedIRFrameRate = m_IRFrameRates[nSel];

			// 카메라의 IR 프레임 레이트 설정
			try
			{
				CameraPtr pCam = m_Device;
				if (pCam)
				{
					INodeMap& nodeMap = pCam->GetNodeMap();

					CEnumerationPtr ptrIRFrameRate = nodeMap.GetNode("IRFrameRate");
					if (IsWritable(ptrIRFrameRate))
					{
						// 모든 엔트리를 검사하여 값이 일치하는 엔트리를 설정
						NodeList_t entries;
						ptrIRFrameRate->GetEntries(entries);
			
						for (size_t i = 0; i < entries.size(); ++i)
						{
							CEnumEntryPtr ptrEntry = entries.at(i);
							if (IsReadable(ptrEntry) && ptrEntry->GetValue() == selectedIRFrameRate)
							{
								ptrIRFrameRate->SetIntValue(ptrEntry->GetValue());
								
								strLog.Format(_T("IR Frame Rate changed to %s"), m_IRFrameRateNames[nSel]);
								AddLog(strLog);
								break;
							}
						}
					}
				}
			}
			catch (Spinnaker::Exception& e)
			{
				strLog.Format(_T("Error occurred while changing IR Frame Rate: %S"), e.what());
				AddLog(strLog);
			}
		}
	}
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::SetTempRange(int nQueryIndex)
{
	CameraPtr camera = m_Device;  // m_Camera는 CameraPtr 타입의 멤버 변수입니다.
	INodeMap& nodeMap = camera->GetNodeMap(); // 카메라의 기능 노드 맵을 가져옵니다.
	int64_t nCurrentCaseValue = m_nCurrentCase[nQueryIndex]; // 설정할 값
	CString strLog;
	bool result = false;

	try 
	{
		CIntegerPtr ptrCurrentCase = nodeMap.GetNode("CurrentCase");
		if (!IsWritable(ptrCurrentCase))
		{
			strLog.Format(_T("Cannot write to 'CurrentCase'."));
			AddLog(strLog);
			return false;
		}

		ptrCurrentCase->SetValue(nCurrentCaseValue); // CurrentCase 파라미터 설정
		strLog.Format(_T("Change CurrentCase = [%lld]"), nCurrentCaseValue);
		AddLog(strLog);
		result = true;
	}
	catch (const Spinnaker::Exception& e)
	{
		strLog.Format(_T("Error in SetTempRange: %S"), e.what());
		AddLog(strLog);
		result = false;
	}

	return result;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnCbnSelchangeCbTempRange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (m_Device == nullptr)
		return;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CB_TEMP_RANGE);
	if (pComboBox)
	{
		SetTempRange(pComboBox->GetCurSel());
		pComboBox->RedrawWindow();
		pComboBox->Invalidate();
		pComboBox->UpdateData();
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnCbnSelchangeCbIrFormat()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if (m_Device == nullptr)
		return;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CB_IR_FORMAT);
	if (pComboBox)
	{
		SetIRFormat((CameraIRFormat)pComboBox->GetCurSel());
		INodeMap& lDeviceParams = m_Device->GetNodeMap();
		SetStreamingCameraParameters(&lDeviceParams);
		pComboBox->RedrawWindow();
		pComboBox->Invalidate();
		pComboBox->UpdateData();
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnAf()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	SetAutoFocus();
}

// =============================================================================
void CMDSSpinnakerSDKDlg::SetIRFormat(CameraIRFormat IRFormat)
{
	m_IRFormat = IRFormat;
}

// =============================================================================
CameraIRFormat CMDSSpinnakerSDKDlg::GetIRFormat()
{
	return m_IRFormat;
}

//=============================================================================
CameraModelList CMDSSpinnakerSDKDlg::GetCameraModelList()
{
	return m_CamModelList;
}

// =============================================================================
// 카메라 종류별 스케일값 적용
double CMDSSpinnakerSDKDlg::GetScaleFactor()
{
	double dScale = 0;
	if (GetIRFormat() == Camera_IRFormat::TEMPERATURELINEAR10MK)
	{
		switch (GetCameraModelList())
		{
		case CAM_MODEL::FT1000:
		case CAM_MODEL::XSC:
		case CAM_MODEL::A300:
		case CAM_MODEL::A400:
		case CAM_MODEL::A500:
		case CAM_MODEL::A700:
		case CAM_MODEL::A615:
		case CAM_MODEL::A50:
			dScale = (0.01f);
			break;
		case CAM_MODEL::Ax5:
			dScale = (0.04f);
			break;
		case CAM_MODEL::BlackFly:
			dScale = 0;
			break;
		default:
			dScale = (0.01f);
			break;
		}
	}
	else if (GetIRFormat() == Camera_IRFormat::TEMPERATURELINEAR100MK)
	{
		switch (GetCameraModelList())
		{
		case CAM_MODEL::FT1000:
		case CAM_MODEL::XSC:
		case CAM_MODEL::A300:
		case CAM_MODEL::A400:
		case CAM_MODEL::A500:
		case CAM_MODEL::A700:
		case CAM_MODEL::A615:
		case CAM_MODEL::A50:
			dScale = (0.1f);
			break;
		case CAM_MODEL::Ax5:
			dScale = (0.4f);
			break;
		case CAM_MODEL::BlackFly:
			dScale = 0;
			break;
		default:
			dScale = (0.1f);
			break;
		}
	}

	return dScale;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_Device == nullptr)
		return;

	SetPauseStreaming(false);
	m_Device->AcquisitionStop();
	
	Sleep(500);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::UpdateCalcParams()
{
	if (!m_bMeasCapable || m_Device == nullptr)
		return;

	if (m_Device->IsStreaming())
	{
		CString strValue;
		
		int nPos = FocusPos(m_Device);
		double dFPS = CameraFPS(m_Device);

		strValue.Format(_T("%d"), nPos);
		SetDlgItemText(IDC_ST_FOCUS_POS, strValue);

		strValue.Format(_T("%.2f"), dFPS);
		//AddLog(strValue);
		SetDlgItemText(IDC_ST_FPS, strValue);

		if (GetIRFormat() == CameraIRFormat::RADIOMETRIC)
		{
			CFloatPtr lB, lF, lO, lR;
			INodeMap& lGenDevice = m_Device->GetNodeMap();

			lR = lGenDevice.GetNode("R");
			lB = lGenDevice.GetNode("B");
			lF = lGenDevice.GetNode("F");

			if (lR && lB && lF)
			{
				double tmpR = lR->GetValue();
				double tmpB = lB->GetValue();
				double tmF = lF->GetValue();

				m_tauPlanckConstants->R = static_cast<int>(tmpR);
				m_tauPlanckConstants->B = static_cast<double>(tmpB);
				m_tauPlanckConstants->F = static_cast<double>(tmF);

			}

			CIntegerPtr lJ0 = lGenDevice.GetNode("J0");
			CFloatPtr lJ1 = lGenDevice.GetNode("J1");
			if (lJ0 && lJ1)
			{
				int64_t tmp = lJ0->GetValue();
				m_tauPlanckConstants->J0 = static_cast<ULONG>(tmp);
				lJ1->GetValue(m_tauPlanckConstants->J1);
			}

			Radiometric_Utility::doUpdateCalcConst(m_objectParams, m_spectralResponseParams, m_tauPlanckConstants);
		}
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::UpdateDeviceOP()
{
	if (m_Device == nullptr)
		return;

	if (GetIRFormat() == CameraIRFormat::RADIOMETRIC)
	{
		if (m_Device->IsStreaming())
		{
			INodeMap& lGenDevice = m_Device->GetNodeMap();

			CFloatPtr lEmissivity = lGenDevice.GetNode("ObjectEmissivity");
			CFloatPtr lExtOptTransm = lGenDevice.GetNode("ExtOpticsTransmission");
			CFloatPtr lRelHum = lGenDevice.GetNode("RelativeHumidity");
			CFloatPtr lDist = lGenDevice.GetNode("ObjectDistance");
			CFloatPtr lAmb = lGenDevice.GetNode("ReflectedTemperature");
			CFloatPtr lAtm = lGenDevice.GetNode("AtmosphericTemperature");
			CFloatPtr lEOT = lGenDevice.GetNode("ExtOpticsTemperature");

			if (lEmissivity && lExtOptTransm && lRelHum && lDist && lAmb && lAtm && lEOT)
			{
				lEmissivity->SetValue(m_objectParams->Emissivity);
				lExtOptTransm->SetValue(m_objectParams->ExtOptTransm);
				lRelHum->SetValue(m_objectParams->RelHum);
				lDist->SetValue(m_objectParams->ObjectDistance);
				lAmb->SetValue(m_objectParams->AmbTemp);
				lAtm->SetValue(m_objectParams->AtmTemp);
				lEOT->SetValue(m_objectParams->ExtOptTemp);
			}
			if (m_bMeasCapable)
			{
				Spinnaker::GenApi::CFloatPtr lAtmTransm = lGenDevice.GetNode("AtmosphericTransmission");
				Radiometric_Utility::doUpdateCalcConst(m_objectParams, m_spectralResponseParams, m_tauPlanckConstants);
				if (lAtmTransm)
				{
					lAtmTransm->SetValue(m_objectParams->AtmTao);
				}
			}
		}
	}
}

// =============================================================================
HBRUSH CMDSSpinnakerSDKDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	UINT nID = pWnd->GetDlgCtrlID();
	HBRUSH hbrRed = m_bRed;
	HBRUSH hbrGreen = m_bGreen;

	// 다이얼로그의 배경 색상을 흰색으로 설정

	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkColor(RGB(255, 255, 255));
		m_hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		return m_hbrBackground;

	case CTLCOLOR_LISTBOX:
		switch (nID)
		{
		case IDC_CAM_LIST:
		case IDC_LOG:
			return (HBRUSH)::GetStockObject(WHITE_BRUSH);
		}
	case CTLCOLOR_STATIC:
		pDC->SetBkMode(TRANSPARENT); // 텍스트 배경을 투명하게 설정
		switch (nID)
		{
		case IDC_LIVE:
			if (m_blink && GetStartRecordingFlag())
			{
				// 녹화 중일 때 깜박이는 효과
				COLORREF color = m_blink ? RGB_RED : RGB_GREEN; // 빨강 또는 초록
				pDC->SetBkColor(color);
				HBRUSH hBrush = CreateSolidBrush(color); // 색상에 맞는 브러시 생성
				return hBrush;
			}
			else
			{
				return (HBRUSH)GetStockObject(WHITE_BRUSH);
			}
		default:
			return (HBRUSH)GetStockObject(WHITE_BRUSH);
			break;

		}
	
	}
	return nullptr;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::ValidateRoiValue(int value, int maxValue, const CString& valueName)
{
	if (value < 0 || value > maxValue)
	{
		CString errorMsg;
		errorMsg.Format(_T("%s 값이 유효하지 않습니다. (0 ~ %d)"), valueName, maxValue);
		AfxMessageBox(errorMsg);
		return false;
	}
	return true;
}

// =============================================================================
int CMDSSpinnakerSDKDlg::GetRoiValue(int controlId)
{
	CString strText;
	GetDlgItemText(controlId, strText);
	return _ttoi(strText);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnRoiSet()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	// 각 ROI 값을 읽어옴
	int ROI_X = GetRoiValue(IDC_ED_ROI_X);
	int ROI_Y = GetRoiValue(IDC_ED_ROI_Y);
	int ROI_Width = GetRoiValue(IDC_ED_ROI_WIDTH);
	int ROI_Height = GetRoiValue(IDC_ED_ROI_HEIGHT);

	// 유효성 검사
	if (!ValidateRoiValue(ROI_X, m_nWidth, _T("ROI X")) ||
		!ValidateRoiValue(ROI_Y, m_nHeight, _T("ROI Y")) ||
		!ValidateRoiValue(ROI_Width, m_nWidth, _T("ROI Width")) ||
		!ValidateRoiValue(ROI_Height, m_nHeight, _T("ROI Height")))
	{
		return;
	}

	// ROI 설정
	m_roi_rect.x = ROI_X;
	m_roi_rect.y = ROI_Y;
	m_roi_rect.width = ROI_Width;
	m_roi_rect.height = ROI_Height;



	// 로그 출력
	CString strLog;
	strLog.Format(_T("ROI : x[%d] y[%d] Width[%d] Height[%d]"), m_roi_rect.x, m_roi_rect.y, m_roi_rect.width, m_roi_rect.height);
	AddLog(strLog);
}

// =============================================================================
void CMDSSpinnakerSDKDlg::OnBnClickedBtnNuc()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SetNUC();
}

// =============================================================================
void CMDSSpinnakerSDKDlg::SetPauseStreaming(bool pause)
{
	m_PauseStreaming = pause;
}

// =============================================================================
// 스트리밍 일시 정지 상태 조회
bool CMDSSpinnakerSDKDlg::GetPauseStreaming() const
{
	return m_PauseStreaming;
}

// =============================================================================
int CMDSSpinnakerSDKDlg::GetSelectedComboBoxNumber(int controlId)
{
	CString strText;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(controlId);
	int curSel = pComboBox->GetCurSel();
	if (curSel != CB_ERR)
	{
		pComboBox->GetLBText(curSel, strText);
		// 정규 표현식을 사용하여 문자열에서 숫자 추출
		std::wregex re(L"\\d+");
		std::wsmatch match;
		std::wstring text = strText.GetString();
		if (std::regex_search(text, match, re))
		{
			return _wtoi(match.str().c_str());
		}
	}
	return -1; // 선택된 항목이 없거나 숫자를 추출하지 못한 경우
}

// =============================================================================
void CMDSSpinnakerSDKDlg::RadioCtrl(UINT radio_Index)
{
	if (m_Device == nullptr)
		return;

	int nIndex = -1;
	switch (radio_Index)
	{
	case IDC_FOCUS_COARSE:
		
		nIndex = 0;
		break;
	case  IDC_FOCUS_FINE:
		nIndex = 1;
		break;
	default:
		nIndex = -1; // 유효하지 않은 인덱스
		break;
	}

	UpdateData(TRUE);
	// 모든 라디오 버튼을 초기화
	const UINT radioIDs[] = { IDC_FOCUS_COARSE, IDC_FOCUS_FINE };

	for (UINT id : radioIDs)
	{
		CButton* pCheck = (CButton*)GetDlgItem(id);
		if (pCheck != nullptr)
		{
			// 현재 인덱스의 라디오 버튼만 체크, 나머지는 체크 해제
			pCheck->SetCheck(id == radio_Index);
		}
	}

	FocusMethod(m_Device, nIndex);
	UpdateData(FALSE);
}

// =============================================================================
CString CMDSSpinnakerSDKDlg::GetLocalIPAddress() 
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) 
	{
		WSACleanup();
		return _T("");
	}

	struct addrinfo hints, * result = NULL;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4 설정
	if (getaddrinfo(hostname, NULL, &hints, &result) != 0) 
	{
		WSACleanup();
		return _T("");
	}

	char ipString[INET_ADDRSTRLEN];  // IPv4 주소를 위한 충분한 크기
	CString localIP;

	for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		if (ptr->ai_family == AF_INET) {  // IPv4 주소 확인
			struct sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
			inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipString, sizeof(ipString));
			localIP = CString(ipString);
			break;
		}
	}

	freeaddrinfo(result);
	WSACleanup();
	return localIP;
}

// =============================================================================
// IP 주소 문자열을 숫자로 변환하는 함수
unsigned long CMDSSpinnakerSDKDlg::ConvertIPStringToNum(const CString& ipStr)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	struct sockaddr_in sa;
	// inet_pton: IPv4 주소의 문자열을 숫자로 변환
	inet_pton(AF_INET, CT2A(ipStr), &(sa.sin_addr));

	WSACleanup();
	return sa.sin_addr.S_un.S_addr;
}

// =============================================================================
std::vector<CString> CMDSSpinnakerSDKDlg::SplitIP(const CString& ip)
{
	std::vector<CString> parts;
	CString strToken;
	int curPos = 0;

	strToken = ip.Tokenize(_T("."), curPos);
	while (curPos != -1) 
	{
		parts.push_back(strToken);
		strToken = ip.Tokenize(_T("."), curPos);
	}
	return parts;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::IsThirdOctetDifferent(const CString& ip1, const CString& ip2)
{
	std::vector<CString> ip1Parts = SplitIP(ip1);
	std::vector<CString> ip2Parts = SplitIP(ip2);

	return ip1Parts.size() == 4 && ip2Parts.size() == 4 && ip1Parts[2] != ip2Parts[2];
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::SetCameraIP(CameraPtr pCam, const CString& newIP)
{
	INodeMap& nodeMap = pCam->GetTLDeviceNodeMap();

	// 강제 IP 설정 노드
	CIntegerPtr ptrForceIPAddress = nodeMap.GetNode("GevDeviceForceIPAddress");
	CIntegerPtr ptrForceSubnetMask = nodeMap.GetNode("GevDeviceForceSubnetMask");
	CIntegerPtr ptrForceGateway = nodeMap.GetNode("GevDeviceForceGateway");
	CCommandPtr ptrForceIP = nodeMap.GetNode("GevDeviceForceIP");

	if (!ptrForceIPAddress || !ptrForceSubnetMask || !ptrForceGateway || !ptrForceIP) 
	{
		AddLog(_T("Failed to retrieve one or more Force IP related nodes."));
		return false;
	}

	// IP 주소 설정
	struct sockaddr_in addr;
	if (inet_pton(AF_INET, CT2A(newIP), &(addr.sin_addr)) != 1) 
	{
		AddLog(_T("Failed to convert new IP address."));
		return false;
	}
	ptrForceIPAddress->SetValue(ntohl(addr.sin_addr.S_un.S_addr));
	CString logMsg;
	//logMsg.Format(_T("Setting IP Address to: %s"), newIP);
	//AddLog(logMsg);

	// 서브넷 마스크 설정 (255.255.255.0)
	inet_pton(AF_INET, "255.255.255.0", &(addr.sin_addr));
	ptrForceSubnetMask->SetValue(ntohl(addr.sin_addr.S_un.S_addr));

	// 게이트웨이 IP 파싱 및 설정
	std::vector<CString> ipOctets = SplitIP(newIP);
	CString gatewayIP = ipOctets[0] + _T(".") + ipOctets[1] + _T(".") + ipOctets[2] + _T(".1");
	if (inet_pton(AF_INET, CT2A(gatewayIP), &(addr.sin_addr)) != 1) 
	{
		AddLog(_T("Failed to convert gateway IP address."));
		return false;
	}
	ptrForceGateway->SetValue(ntohl(addr.sin_addr.S_un.S_addr));
	//logMsg.Format(_T("Setting Gateway to: %s"), gatewayIP);
	//AddLog(logMsg);


	// 설정된 값 확인
	uint32_t setIPAddress = ptrForceIPAddress->GetValue();
	uint32_t setSubnetMask = ptrForceSubnetMask->GetValue();
	uint32_t setGateway = ptrForceGateway->GetValue();
	CString ipStr = ConvertIPToString(setIPAddress);
	logMsg.Format(_T("Confirmed IP Address: %s"), ipStr);
	AddLog(logMsg);
	ipStr = ConvertIPToString(setSubnetMask);
	logMsg.Format(_T("Confirmed Subnet Mask: %s"), ipStr);
	AddLog(logMsg);
	ipStr = ConvertIPToString(setGateway);
	logMsg.Format(_T("Confirmed Gateway: %s"), ipStr);
	AddLog(logMsg);

	// Force IP 명령 실행
	ptrForceIP->Execute();
	AddLog(_T("IP address, subnet mask, and gateway forced successfully."));
	return true;
}

void CMDSSpinnakerSDKDlg::OnBnClickedBtnChangeip()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 로컬 IP 주소 얻기
	CString localIP = GetLocalIPAddress();
	if (localIP.IsEmpty()) {
		AddLog(_T("Failed to retrieve local IP address."));
		return;
	}

	CameraPtr pCam = nullptr;
	bool isCameraFound = false;
	CIntegerPtr ptrIP;

	CString ipStr;

	// 카메라 리스트 순회
	for (int i = 0; i < m_CamList.GetSize(); i++) {
		pCam = m_CamList.GetByIndex(i);
		if (!pCam || !pCam->IsValid()) {
			continue; // 유효하지 않은 카메라는 건너뜀
		}
		INodeMap& nodeMap = pCam->GetTLDeviceNodeMap();
		ptrIP = nodeMap.GetNode("GevDeviceIPAddress");
		if (IsReadable(ptrIP))
		{
			int64_t nValue = ptrIP->GetValue();
			ipStr = ConvertIPToString(nValue);
		}
		if (IsSameNetwork(localIP, ipStr)) 
		{
			isCameraFound = true;
			break; // 일치하는 카메라 찾으면 반복 중단
		}
	}

	if (!isCameraFound)
	{
		AddLog(_T("No camera is on the same network segment as the local machine."));
		return;
	}

	// 카메라의 현재 IP 주소 얻기
	INodeMap& nodeMap = pCam->GetTLDeviceNodeMap();
	CString currentIPStr = ConvertIPToString(ptrIP->GetValue());
	std::vector<CString> currentIPOctets = SplitIP(currentIPStr);

	// 사용자로부터 네 번째 옥텟 값 받아오기
	CString fourthOctet;
	GetDlgItemText(IDC_ED_SETIP, fourthOctet);
	std::vector<CString> localIPOctets = SplitIP(localIP);
	// 새 IP 주소 생성
	if (localIPOctets.size() == 4 && currentIPOctets.size() == 4) 
	{
		CString newIP = localIPOctets[0] + _T(".") + localIPOctets[1] + _T(".") + localIPOctets[2] + _T(".") + fourthOctet;

		// 카메라 IP 변경 시도
		if (!SetCameraIP(pCam, newIP)) 
		{
			CString strLog;
			strLog.Format(_T("Failed to change camera IP to %s."), newIP);
			AddLog(strLog);
		}
		else 
		{
			CString strLog;
			strLog.Format(_T("Camera IP changed successfully to %s."), newIP);
			AddLog(strLog);

			m_CamListBox.ResetContent();
			UpdateWindow();
			FindCamera();
		}
	}
	else 
	{
		CString strLog = _T("Invalid IP address format.");
		AddLog(strLog);
	}
}


CString CMDSSpinnakerSDKDlg::GetLocalNetworkBaseAddress(const CString& cameraIP)
{
	ULONG buflen = sizeof(IP_ADAPTER_ADDRESSES);
	std::vector<BYTE> buffer(buflen);

	// Get the adapter addresses
	ULONG ret = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, (IP_ADAPTER_ADDRESSES*)&buffer[0], &buflen);
	if (ret == ERROR_BUFFER_OVERFLOW) {
		buffer.resize(buflen);
		ret = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, (IP_ADAPTER_ADDRESSES*)&buffer[0], &buflen);
	}

	if (ret == NO_ERROR) {
		IP_ADAPTER_ADDRESSES* adapter = (IP_ADAPTER_ADDRESSES*)&buffer[0];
		while (adapter) {
			IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress;
			while (address) {
				if (address->Address.lpSockaddr->sa_family == AF_INET) {
					sockaddr_in* sa_in = (sockaddr_in*)address->Address.lpSockaddr;
					char ip[INET_ADDRSTRLEN];
					struct sockaddr_in sa;
					inet_ntop(AF_INET, &(sa.sin_addr), ip, INET_ADDRSTRLEN);
					CString currentIP = CA2T(ip);  // char*를 CString으로 변환

					// Compare the current IP with the camera IP
					if (IsSameNetwork(currentIP, cameraIP)) {
						return currentIP.Left(currentIP.ReverseFind('.'));
					}
				}
				address = address->Next;
			}
			adapter = adapter->Next;
		}
	}

	return _T("");
}

bool CMDSSpinnakerSDKDlg::IsSameNetwork(const CString& ip1, const CString& ip2)
{
	// Implement the actual network comparison here.
	// This function is a placeholder and should be implemented properly.
	return ip1.Left(ip1.ReverseFind('.')) == ip2.Left(ip2.ReverseFind('.'));
}

//=============================================================================
void CMDSSpinnakerSDKDlg::AddBufferToQueue(ImagePtr buffer)
{
	std::lock_guard<std::mutex> lock(bufferMutex);

	ImagePtr copiedBuffer = CopyBuffer(buffer);
	if (!copiedBuffer)
	{
		CString logMessage;
		logMessage.Format(_T("Failed to copy buffer."));
		AddLog(logMessage);
		return;
	}

	int height = copiedBuffer->GetHeight();
	bufferQueue.push(copiedBuffer);

	bufferNotEmpty.notify_one();
}

/**
 * @brief 주어진 PvBuffer 객체의 복사본을 생성합니다.
 *
 * 이 함수는 제공된 PvBuffer 객체의 깊은 복사본을 생성합니다. 원본 버퍼의 데이터를 복사하여
 * 새로운 PvBuffer 객체에 첨부합니다. 복사가 성공적으로 완료되지 않으면, 할당된 메모리를 해제하고
 * nullptr을 반환합니다.
 *
 * @param originalBuffer 복사할 원본 PvBuffer 객체입니다.
 * @return PvBuffer* 원본 버퍼의 데이터를 포함하는 새로운 PvBuffer 객체입니다.
 */
 //=============================================================================
ImagePtr CMDSSpinnakerSDKDlg::CopyBuffer(ImagePtr originalBuffer)
{
	// 원본 이미지에서 데이터를 검사합니다.
	if (!originalBuffer)
	{
		CString logMessage;
		logMessage.Format(_T("Original buffer data is invalid."));
		AddLog(logMessage);
		return nullptr;
	}

	// 원본 이미지 데이터를 깊은 복사합니다.
	ImagePtr copiedBuffer = Image::Create();

	// DeepCopy the original image into the new image
	copiedBuffer->DeepCopy(originalBuffer);

	if (!copiedBuffer)
	{
		CString logMessage;
		logMessage.Format(_T("Failed to copy buffer."));
		AddLog(logMessage);
		return nullptr;
	}

	return copiedBuffer;
}

//=============================================================================
/*
 [condition_variable]
 멀티스레드 프로그래밍에서 스레드 간의 효율적인 통신과 동기화를 위한 로직이다.
 이를 통해 특정 조건이 만족될 때까지 스레드를 대기시키고, 조건이 만족되면 대기 중인 스레드를 동작시킨다.

*@brief 큐에서 PvBuffer 객체를 가져옵니다.
*
* 이 함수는 Q에 버퍼가 있을 때까지 대기한 후, Q에서 버퍼를 가져와 제거.
* 큐 작업 중 스레드 안전성을 보장하기 위해 뮤텍스 사용
*
* @return PvBuffer * Q에서 가져온 PvBuffer
*/
//=============================================================================
ImagePtr CMDSSpinnakerSDKDlg::GetBufferFromQueue()
{
	// 큐 작업 중 스레드 안전성을 보장하기 위해 뮤텍스를 잠급니다.
	std::unique_lock<std::mutex> lock(bufferMutex);

	// 큐가 비어 있지 않을 때까지 대기합니다. 대기 중에는 잠금이 해제됩니다.
	bufferNotEmpty.wait(lock, [this] { return !bufferQueue.empty(); });

	// 큐의 맨 앞에서 버퍼를 가져와 큐에서 제거합니다.
	ImagePtr buffer = bufferQueue.front();
	bufferQueue.pop();

	// 버퍼에서 이미지를 가져와 높이를 로그로 남깁니다.
	//int height = buffer->GetHeight();
	//CString logMessage;
	//logMessage.Format(_T("Height: %d , Buffer retrieved from queue. Address: %p,"), height, buffer);
	//AddLog(logMessage);

	return buffer;
}

//=============================================================================
void CMDSSpinnakerSDKDlg::ClearQueue()
{
	std::lock_guard<std::mutex> lock(bufferMutex);
	while (!bufferQueue.empty())
	{
		ImagePtr buffer = bufferQueue.front();
		bufferQueue.pop();
	}
}

//=============================================================================
// Q에 담겨있는 버퍼 갯수 
int CMDSSpinnakerSDKDlg::GetQueueBufferCount()
{
	std::lock_guard<std::mutex> lock(bufferMutex);

	return (int)bufferQueue.size();
}

// =============================================================================
void CMDSSpinnakerSDKDlg::SetStartRecordingFlag(bool bFlag)
{
	m_bStartRecording = bFlag;


	if (bFlag)
	{
		m_bSaveAsSEQ = true;
		m_iCurrentSEQImage = 0;
	}
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::GetStartRecordingFlag()
{
	return m_bStartRecording;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::StartRecording(int frameWidth, int frameHeight, double frameRate)
{
	CString strLog = _T("");

	if (m_isRecording)
	{
		strLog.Format(_T("---------Camera[%d] Already recording"));
		AddLog(strLog);

		return false; // 이미 녹화 중이므로 false 반환
	}
	m_isRecording = true;

	// 녹화 이미지 생성을 위한 쓰레드 시작
	std::thread recordThread(&CMDSSpinnakerSDKDlg::RecordThreadFunction, this, frameRate);
	recordThread.detach();  // 안전한 쓰레드 종료 관리

	return true; // 녹화 시작 성공 반환
}

// =============================================================================
void CMDSSpinnakerSDKDlg::RecordThreadFunction(double frameRate)
{
	while (GetStartRecordingFlag())
	{
		if(GetQueueBufferCount() > 0)
			SaveSEQ(m_nWidth, m_nHeight);
			
		std::this_thread::sleep_for(std::chrono::milliseconds(0));
	}
}

// =============================================================================
void CMDSSpinnakerSDKDlg::StopRecording()
{
	if (!m_isRecording)
	{
		return;
	}

	CString cstrFilePath(m_SEQfilePath.c_str());
	CString strLog = _T("");

	if (m_SEQFile.m_hFile != CFile::hFileNull)
	{
		m_SEQFile.Close();
	}
	 
	m_isRecording = false;
	m_bSaveAsSEQ = false;
	SetStartRecordingFlag(false);
	m_blink = false;
	m_lbLive.Invalidate();
	m_lbLive.UpdateWindow();

}

// =============================================================================
bool CMDSSpinnakerSDKDlg::StartSEQRecording(CString strfilePath)
{

	if (m_SEQFile.m_hFile != CFile::hFileNull)
	{
		m_SEQFile.Close();  // 이미 열려 있는 파일이 있다면 닫기
	}

	m_SEQfilePath = CT2A(strfilePath);  // 파일 경로 저장
	CFileException e;

	try
	{
		m_SEQFile.Open(strfilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary);
	}

	catch (CFileException* e)
	{
		std::cerr << "Failed to open file: " << e->m_cause << "\n";
		e->Delete();
		m_bSaveAsSEQ = false;
		return false;
	}
	return true;
}


//=============================================================================
// Trig flag bits - description
// Bit 15: 1=This trig info is relevant 
//         0=Look for trig info in pixel data instead
// Bit 5:  1=Stop trig type
// Bit 4:  1=Start trig type
// Bit 3:  0=Device 1=Serial port trig (or LPT)
// Bit 2:  0=TTL       1=OPTO 
// Bit 1:  0=Negative  1=Positive
// Bit 0:  0=No trig   1=Trigged
//=============================================================================
void CMDSSpinnakerSDKDlg::SaveSEQ(int nWidth, int nHeight)
{
	auto currentTime = std::chrono::system_clock::now();
	bool bFileCreateFlag = false;

	// 헤더가 포함된 데이터인지 유효성 체크

	ImagePtr pBuf;
	pBuf = GetBufferFromQueue();
	
	int Height = 0;
	ImagePtr lImage = m_Device->GetNextImage();
	Height = lImage->GetHeight();

	// 헤더 데이터가 포함되지 않으면 바로 리턴한다
	if (Height == 480 || Height == 348)
	{
		StopRecording();
		return;
	}

	// 로직 첫 진입 시 파일 만들기 
	if (m_iCurrentSEQImage == 0)
	{
		std::time_t nowAsTimeT = std::chrono::system_clock::to_time_t(currentTime);
		std::tm localTime;
		localtime_s(&localTime, &nowAsTimeT);
		char timeStr[50];
		strftime(timeStr, sizeof(timeStr), "_%Y_%m_%d_%H_%M_%S", &localTime);
		CString videoDataPath = _T("VideoData\\");  // 폴더 구분자를 명확히 추가
		CString SavePath = SetFolderPath(videoDataPath);
		CString fullSavePath = SetFolderPath(videoDataPath);
		std::string strSavePath = CString2str(fullSavePath);

		if (!PathIsDirectory(fullSavePath))
		{
			SHCreateDirectoryEx(NULL, fullSavePath, NULL);
		}

		std::string strSeqName = strSavePath + "SEQ" + std::string(timeStr) + ".seq";
		CString strSeqFileName = str2CString(strSeqName.c_str());

		bFileCreateFlag = StartSEQRecording(strSeqFileName);
		if (!bFileCreateFlag)
			return;
	}

	// buffer to SEQ 로직
	int nPixelSize = nHeight * nWidth * 2;
	uint8_t* pSrc = nullptr;
	pSrc = (uint8_t*)pBuf->GetData();

	if (!pSrc || *pSrc == 0) return;

	if (m_bSaveAsSEQ)
	{
		FPGA_HEADER* pFPGA;
		FLIRFILEHEAD* pHeader;
		FLIRFILEINDEX* pIndex;
		pFPGA = (FPGA_HEADER*)&pSrc[nPixelSize];
		WORD trgflags = 0;
		GEOMETRIC_INFO_T geom;
		DWORD dwNumUsedIndex = 0;

		memset(&geom, 0, sizeof(GEOMETRIC_INFO_T));

		geom.firstValidX = 0;
		geom.firstValidY = 0;
		geom.lastValidX = nWidth - 1;
		geom.lastValidY = nHeight - 1;
		geom.imageHeight = nHeight;
		geom.imageWidth = nWidth;
		geom.pixelSize = 2;

		pHeader = (FLIRFILEHEAD*)&pSrc[nPixelSize + sizeof(FPGA_HEADER)];
		pIndex = (FLIRFILEINDEX*)&pSrc[nPixelSize + sizeof(FPGA_HEADER) + sizeof(FLIRFILEHEAD)];
		dwNumUsedIndex = pHeader->dwNumUsedIndex;

		ULONG offs = sizeof(FLIRFILEHEAD) + (sizeof(FLIRFILEINDEX) * dwNumUsedIndex);

		for (int i = 0; i < (int)dwNumUsedIndex; i++)
		{
			pIndex[i].dwChecksum = 0;
			if (pIndex[i].wMainType == FFF_TAGID_Pixels)
			{
				pIndex[i].dwDataSize = nPixelSize + sizeof(GEOMETRIC_INFO_T);
			}
			pIndex[i].dwDataPtr = offs;
			offs += pIndex[i].dwDataSize;
		}
		if (pFPGA == nullptr || pHeader == nullptr || pIndex == nullptr) 
		{
			AddLog(_T("Data Fail"));
			return;
		}

		try
		{
			if (!pHeader || !pIndex) {
				AddLog(_T("pHeader, pIndex NULL"));
				return;
			}

			// Write to file
			if (m_SEQFile.m_hFile != CFile::hFileNull)  // 파일이 열려 있는지 확인
			{
				m_SEQFile.SeekToEnd(); // Ensure the file pointer is at the correct position before writing
				m_SEQFile.Write(pHeader, sizeof(FLIRFILEHEAD));
				m_SEQFile.Write(pIndex, sizeof(FLIRFILEINDEX) * dwNumUsedIndex);
			}

			for (int i = 0; i < (int)dwNumUsedIndex; i++)
			{
				if (pIndex[i].wMainType == FFF_TAGID_BasicData)
				{
					PBYTE pData;
					BI_DATA_T* pBI;

					pData = (PBYTE)&pIndex[dwNumUsedIndex];
					pBI = (BI_DATA_T*)pData;

					if (!pData) {
						AddLog(_T("pData null"));
						return;
					}

					pBI->GeometricInfo.pixelSize = 2;
					pBI->GeometricInfo.imageHeight = nHeight;
					pBI->GeometricInfo.imageWidth = nWidth;
					pBI->ImageInfo.imageTime = (unsigned long)m_ts;
					pBI->ImageInfo.imageMilliTime = m_ms;
					pBI->ImageInfo.timeZoneBias = m_tzBias;

					pBI->PresentParameters.level = m_Level;
					pBI->PresentParameters.span = m_Span;

					// Transfer trig info - if any
					if ((pFPGA->dp1_trig_type | pFPGA->dp2_trig_type) &
						(FPGA_TRIG_TYPE_MARK |
							FPGA_TRIG_TYPE_MARK_START |
							FPGA_TRIG_TYPE_MARK_STOP)
						)
					{

						trgflags = 0x8000; // Trig info is relevant 
						if ((pFPGA->dp1_trig_type | pFPGA->dp2_trig_type) & FPGA_TRIG_TYPE_MARK)
							trgflags |= 0x0001; // Normal trig mark
						if ((pFPGA->dp1_trig_type | pFPGA->dp2_trig_type) & FPGA_TRIG_TYPE_MARK_START)
							trgflags |= 0x0010; // Start trig type
						if ((pFPGA->dp1_trig_type | pFPGA->dp2_trig_type) & FPGA_TRIG_TYPE_MARK_STOP)
							trgflags |= 0x0020; // Stop trig type
					}

					pBI->ImageInfo.trigFlags = trgflags;
					pBI->ImageInfo.trigCount = m_TrigCount;
					pBI->ImageInfo.trigHit = 0;
					pBI->ImageInfo.trigInfoType = 1;

					// Save a copy of geometric data
					memcpy(&geom, pData, sizeof(GEOMETRIC_INFO_T));

					if (m_SEQFile.m_hFile != CFile::hFileNull)  // 파일이 열려 있는지 확인
					{
						// Write the modified Basic Image data block to file
						m_SEQFile.Write(pData, pIndex[i].dwDataSize);
					}
				}
			}

			if (m_SEQFile.m_hFile != CFile::hFileNull)  // 파일이 열려 있는지 확인
			{
				m_SEQFile.Write(&geom, sizeof(GEOMETRIC_INFO_T));
				m_SEQFile.Write(pSrc, nPixelSize);
				m_iCurrentSEQImage++;
			}
		}
		catch (CFileException* e)
		{
			return;
		}
	}
}

// =============================================================================
CString CMDSSpinnakerSDKDlg::str2CString(const char* _str)
{
	CString str2;
	str2 = (CString)_str;

	return str2;

}
// =============================================================================
std::string CMDSSpinnakerSDKDlg::CString2str(CString _str)
{

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString(_str);

	// Construct a std::string using the LPCSTR input
	std::string s(pszConvertedAnsiString);

	return s;
}

// =============================================================================
CString CMDSSpinnakerSDKDlg::SetFolderPath(CString _str)
{
	// 프로그램 실행 경로에 지정된 문자열을 연결하는 함수.
	TCHAR path1[_MAX_DIR];
	GetModuleFileName(NULL, path1, sizeof path1);

	CString strPath = path1;
	CString rootpath, path, Lfilepath, Rfilepath;
	int i = strPath.ReverseFind('\\');
	strPath = strPath.Left(i);
	CString str;
	str.Format(_T("%s\\%s"), (LPCTSTR)strPath, _str);

	//디렉토리 존재 여부 확인
	DWORD dwAttrib = GetFileAttributes(str);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES)
	{
		// 디렉토리가 존재하지 않는 경우, 디렉토리를 먼저 생성
		CString directoryPath = str.Left(str.ReverseFind('\\'));
		if (!PathIsDirectory(directoryPath))
		{
			SHCreateDirectoryEx(NULL, directoryPath, NULL);
		}
	}

	return str;
}

// =============================================================================
void CMDSSpinnakerSDKDlg::SetMouseImageSaveFlag(bool bFlag)
{
	m_bMouseImageSave = bFlag;
}
// =============================================================================
bool CMDSSpinnakerSDKDlg::GetMouseImageSaveFlag()
{
	return m_bMouseImageSave;
}

// =============================================================================
std::string CMDSSpinnakerSDKDlg::GenerateFileNameWithTimestamp(const std::string& basePath, const std::string& prefix, const std::string& extension)
{
	// 현재 시간 정보 가져오기
	auto currentTime = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::to_time_t(currentTime);
	struct tm localTime;
	localtime_s(&localTime, &now);

	// 시간 정보를 이용하여 파일 이름 생성
	char timeStr[50];
	strftime(timeStr, sizeof(timeStr), "_%Y_%m_%d_%H_%M_%S", &localTime);

	// 최종 파일 경로 생성
	std::string filePath = basePath + prefix + timeStr + extension;

	return filePath;
}

// =============================================================================
bool CMDSSpinnakerSDKDlg::SaveImage(const cv::Mat& image)
{

	CString videoDataPath = _T("ImageData\\");  // 폴더 구분자를 명확히 추가
	CString SavePath = SetFolderPath(videoDataPath);
	CString fullSavePath = SetFolderPath(videoDataPath);
	std::string strSavePath = CString2str(fullSavePath);

	if (!PathIsDirectory(fullSavePath))
	{
		SHCreateDirectoryEx(NULL, fullSavePath, NULL);
	}

	std::string basePath = strSavePath; // 이미지 저장 경로
	std::string prefix = "Overlayimage"; // 파일 이름 접두사
	std::string extension = ".bmp"; // 확장자

	// 파일 이름 생성
	std::string filePath = GenerateFileNameWithTimestamp(basePath, prefix, extension);

	// 이미지 저장
	if (!cv::imwrite(filePath, image))
	{
		// 오류 처리
		return false;
	}

	// 성공적으로 저장한 경우 로그 출력
	CString strLog;
	std::wstring filePathW(filePath.begin(), filePath.end());
	strLog.Format(_T("Completed writeback to image file: \n%s"), filePathW.c_str());
	AddLog(strLog);

	return true;
}