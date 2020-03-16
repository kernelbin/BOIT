#include<Windows.h>
#include<gdiplus.h>
#pragma comment(lib, "gdiplus.lib")  

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders  
    UINT  size = 0;         // size of the image encoder array in bytes  

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure  

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure  

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success  
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure  
}


extern "C" BOOL SaveHDCToFile(HDC hDC, LPRECT lpRect, WCHAR FilePath[])
{
    BOOL bRet = FALSE;
    int nWidth = lpRect->right - lpRect->left;
    int nHeight = lpRect->bottom - lpRect->top;

    //将目标区域贴图到内存BITMAP  
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
    SelectObject(memDC, hBmp);
    BitBlt(memDC, lpRect->left, lpRect->top, nWidth, nHeight,
        hDC, 0, 0, SRCCOPY);

    //保存成文件  
    {
        //L"image/bmp" L"image/jpeg"  L"image/gif" L"image/tiff" L"image/png"  
        CLSID pngClsid;
        GetEncoderClsid(L"image/jpeg", &pngClsid);//此处以BMP为例，其它格式选择对应的类型，如JPG用L"image/jpeg"   

        Gdiplus::Bitmap* pbmSrc = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);
        if (pbmSrc->Save(FilePath, &pngClsid) == Gdiplus::Ok)
        {
            bRet = TRUE;
        }
        delete pbmSrc;
    }

    //清理工作  
    SelectObject(memDC, (HBITMAP)NULL);
    DeleteDC(memDC);
    DeleteObject(hBmp);

    return bRet;
}

extern "C" BOOL InitGDIPlus()
{
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    return TRUE;
}

extern "C" BOOL CleanupGDIPlus()
{
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return TRUE;
}
