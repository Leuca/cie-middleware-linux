/*
 *  PdfSignatureGenerator.cpp
 *  SignPoDoFo
 *
 *  Created by svp on 26/05/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "PdfSignatureGenerator.h"
#include "PdfVerifier.h"
#include "UUCLogger.h"

#define SINGNATURE_SIZE 10000
#define MAX_TMP 1000
#define FONT_NAME "DejaVu Sans"
#define FONT_SIZE 12.0

#ifdef CreateFont
#undef CreateFont
#endif

#ifdef GetObject
#undef GetObject
#endif

int GetNumberOfSignatures(PdfMemDocument* pPdfDocument);

USE_LOG;

PdfSignatureGenerator::PdfSignatureGenerator()
: m_pPdfDocument(NULL), m_pSignatureField(NULL), m_pSignOutputDevice(NULL), m_pFinalOutDevice(NULL),
m_pMainDocbuffer(NULL), m_pSignDocbuffer(NULL) {}

PdfSignatureGenerator::~PdfSignatureGenerator()
{
	if(m_pPdfDocument)
		delete m_pPdfDocument;
	
	if(m_pSignatureField)
		delete m_pSignatureField;
	
	if(m_pSignOutputDevice)
		delete m_pSignOutputDevice;
	
	if(m_pFinalOutDevice)
		delete m_pFinalOutDevice;
	
	if(m_pMainDocbuffer)
		delete m_pMainDocbuffer;
	
	if(m_pSignDocbuffer)
		delete m_pSignDocbuffer;
	
}

int PdfSignatureGenerator::Load(const char* pdf, int len)
{
	if(m_pPdfDocument)
		delete m_pPdfDocument;
	
	try
	{
		printf("Pdf len: %d\n", len);

		m_pPdfDocument = new PdfMemDocument();
		m_pPdfDocument->LoadFromBuffer(pdf, len, false);

		printf("OK m_pPdfDocument");

		m_actualLen = len;
		
		return PDFVerifier::GetNumberOfSignatures(m_pPdfDocument);
	}
    catch(::PoDoFo::PdfError& err)
    {
        return -2;
    }
	catch (...)
	{
		return -1;
	}
}

void PdfSignatureGenerator::InitSignature(int pageIndex, const char* szReason, const char* szReasonLabel, const char* szName, const char* szNameLabel, const char* szLocation, const char* szLocationLabel, const char* szFieldName, const char* szSubFilter)
{
	LOG_DBG((0, "quella con tutti 0\n", ""));
	InitSignature(pageIndex, 0, 0, 0, 0, szReason, szReasonLabel, szName, szNameLabel, szLocation, szLocationLabel, szFieldName, szSubFilter);
}

void PdfSignatureGenerator::InitSignature(int pageIndex, float left, float bottom, float width, float height, const char* szReason, const char* szReasonLabel, const char* szName, const char* szNameLabel, const char* szLocation, const char* szLocationLabel, const char* szFieldName, const char* szSubFilter)
{
	LOG_DBG((0,"quella senza tutti 0\n", ""));
	InitSignature(pageIndex, left, bottom, width,  height, szReason, szReasonLabel, szName, szNameLabel, szLocation, szLocationLabel, szFieldName, szSubFilter, NULL, NULL, NULL, NULL);
}

void PdfSignatureGenerator::InitSignature(int pageIndex, float left, float bottom, float width, float height, const char* szReason, const char* szReasonLabel, const char* szName, const char* szNameLabel, const char* szLocation, const char* szLocationLabel, const char* szFieldName, const char* szSubFilter, const char* szImagePath, const char* szDescription, const char* szGraphometricData, const char* szVersion)
{
	printf("--> InitSignature %d, %f, %f, %f, %f, %s, %s, %s, %s, %s, %s, %s, %s\n", pageIndex, left, bottom, width, height, szReason, szName, szLocation, szFieldName, szSubFilter, szImagePath, szGraphometricData, szVersion);
    //LOG_DBG((0, "--> InitSignature", ""));
    
	if(m_pSignatureField)
		delete m_pSignatureField;

	PdfPage* pPage = m_pPdfDocument->GetPage(pageIndex);
    PdfRect cropBox = pPage->GetCropBox();

    float cropBoxWidth = cropBox.GetWidth();
    float cropBoxHeight = cropBox.GetHeight();
    
    float left0 = left * cropBoxWidth;
    float bottom0 = cropBoxHeight - (bottom * cropBoxHeight);
    
    float width0 = width * cropBoxWidth;
    float height0 = height * cropBoxHeight;
    
	LOG_DBG((0, "InitSignature", "m_actualLen %d", m_actualLen));
	int fulllen = m_actualLen * 2 + SINGNATURE_SIZE * 2 + (szGraphometricData ? (strlen(szGraphometricData) + strlen(szVersion) + 100) : 0);

    printf("pdf rect: %f, %f, %f, %f\n", left0, bottom0, width0, height0);

	PdfRect rect(left0, bottom0, width0, height0);

	LOG_DBG((0, "InitSignature", "PdfSignatureField"));

	m_pSignatureField = new PdfSignatureField(pPage, rect, m_pPdfDocument);

	LOG_DBG((0, "InitSignature", "PdfSignatureField OK"));

	// This is the card holder's name
	// Should go in /Name, goes in /Reason
	if(szReason && szReason[0])
	{
		PdfString name(szReason);
		m_pSignatureField->SetSignatureReason(name);
	}

	// /T: SignatureN
	if(szFieldName && szFieldName[0])
	{
		// This corresponds to /T
		PdfString fieldName = PdfString(szFieldName);
		m_pSignatureField->SetFieldName(fieldName);
	}
	
	LOG_DBG((0, "InitSignature", "Name OK"));

	if(szLocation && szLocation[0])
	{
		PdfString location(szLocation);
		m_pSignatureField->SetSignatureLocation(location);
	}

	LOG_DBG((0, "InitSignature", "szLocation OK"));

	PdfDate now;
	m_pSignatureField->SetSignatureDate(now);

	LOG_DBG((0, "InitSignature", "Date OK"));

	// This is the signature date
	// Should go in /M goes in /Name
	if(szName && szName[0])
	{
		m_pSignatureField->GetSignatureObject()->GetDictionary().AddKey(PdfName("Name"),
				PdfObject(PdfString(szName)));
	}

	LOG_DBG((0, "InitSignature", "szName OK"));

	// Create graphical signature with stamp if we have a picture
	if(width * height > 0)
	{
		PdfXObject sigXObject (rect, m_pPdfDocument);
		PdfPainter painter;

		try
		{
			PdfImage image(m_pPdfDocument);
			double scale;

			// Generate signature string
			std::string signatureStamp;

			// Append date
			if(szName && szName[0])
				signatureStamp.append(szName);

			// Append name
			if(szReason && szReason[0])
			{
				signatureStamp.append("\n");
				signatureStamp.append(szReason);
			}

			image.LoadFromFile(szImagePath);
			// Using a single ratio should avoid squeezing images
			// not entirely shure though
			scale = width0 / image.GetWidth();

			// Draw signature
			painter.SetPage(&sigXObject);
			painter.Save();
			painter.Restore();
			painter.DrawImage(left0, bottom0, &image, scale, scale);

			// Create signature stamp
			PdfFont* font = m_pPdfDocument->CreateFont(FONT_NAME, false,
					PdfEncodingFactory::GlobalWinAnsiEncodingInstance(),
					PdfFontCache::eFontCreationFlags_AutoSelectBase14, false);
			printf("Font found: code %s\n", font->GetIdentifier().GetName().c_str());
			painter.SetFont(font);
			font->SetFontSize(FONT_SIZE);
			painter.DrawMultiLineText(rect, PdfString(signatureStamp));

			m_pSignatureField->SetAppearanceStream(&sigXObject);

			LOG_DBG((0, "InitSignature", "SetAppearance OK"));
		}
		catch( PdfError & error ) 
		{
			LOG_ERR((0, "InitSignature", "SetAppearance error: %s, %s", PdfError::ErrorMessage(error.GetError()), error.what()));			
			if(painter.GetPage())
			{
				try
				{
					painter.FinishPage();
				}
				catch(...)
				{
				}
			}
		}
		catch( PdfError * perror ) 
		{
			LOG_ERR((0, "InitSignature", "SetAppearance error2: %s, %s", PdfError::ErrorMessage(perror->GetError()), perror->what()));			
		}
		catch(std::exception& ex)
		{
			LOG_ERR((0, "InitSignature", "SetAppearance std exception, %s", ex.what()));			
		}
		catch(std::exception* pex)
		{
			LOG_ERR((0, "InitSignature", "SetAppearance std exception2, %s", pex->what()));			
		}
		catch(...)
		{
			LOG_ERR((0, "InitSignature", "SetAppearance unknown error"));			
		}

		painter.FinishPage();
		// Remove the font we embedded
		m_pPdfDocument->GetAcroForm()->GetObject()->GetDictionary().RemoveKey(PdfName("DA"));
		m_pPdfDocument->GetAcroForm()->GetObject()->GetDictionary().RemoveKey(PdfName("DR"));
	}

	// Set SubFilter
	if(szSubFilter && szSubFilter[0])
	{
		m_pSignatureField->GetSignatureObject()->GetDictionary().AddKey("SubFilter",
					PdfName(szSubFilter));
	}

	// Add /SigFlags
	pdf_int64 flags = 3;
	m_pPdfDocument->GetAcroForm()->GetObject()->GetDictionary().AddKey(PdfName("SigFlags"),
			PdfObject(flags));

	// Keep trying to write the final document until we don't catch an error.
	// Double buffer lenght on error.
	m_pSignDocbuffer = NULL;
	while (!m_pSignDocbuffer) {
		try
		{
			LOG_DBG((0, "InitSignature", "fulllen %d", fulllen));

			m_pSignDocbuffer = new char[fulllen];

			LOG_DBG((0, "InitSignature", "m_pSignDocbuffer %d", fulllen));

			m_pFinalOutDevice = new PdfOutputDevice(m_pSignDocbuffer, fulllen);
			m_pSignOutputDevice = new PdfSignOutputDevice(m_pFinalOutDevice);

			LOG_DBG((0, "InitSignature", "buffers OK %d", fulllen));

			m_pSignOutputDevice->SetSignatureSize(SINGNATURE_SIZE);

			LOG_DBG((0, "InitSignature", "SetSignatureSize OK %d", SINGNATURE_SIZE));

			m_pSignatureField->SetSignature(*m_pSignOutputDevice->GetSignatureBeacon());

			m_pPdfDocument->Write(m_pSignOutputDevice);
		}
		catch(::PoDoFo::PdfError err)
		{
			if(m_pSignDocbuffer) {
			        delete m_pSignDocbuffer;
			        m_pSignDocbuffer = NULL;
			}
			if(m_pFinalOutDevice)
				delete m_pFinalOutDevice;
			if(m_pSignOutputDevice)
				delete m_pSignOutputDevice;

			LOG_DBG((0, "PdfError", "what %s", err.what()));
			printf("Allocating bigger buffer...\n");

			fulllen *= 2;
		}
	}
}

void PdfSignatureGenerator::GetBufferForSignature(UUCByteArray& toSign)
{
	int len = m_pSignOutputDevice->GetLength() * 2;
	char* buffer = new char[len];
	int nRead;

	m_pSignOutputDevice->AdjustByteRange();
	LOG_DBG((0, "SetSignature", "AdjustByteRange OK"));

	m_pSignOutputDevice->Seek(0);

	nRead = m_pSignOutputDevice->ReadForSignature(buffer, len);
	if(nRead == -1)
		throw nRead;

	toSign.append((BYTE*)buffer, nRead);

	delete buffer;
}

void PdfSignatureGenerator::SetSignature(const char* signature, int len)
{
	PdfData signatureData(signature, len);
	m_pSignOutputDevice->SetSignature(signatureData);
	m_pSignOutputDevice->Flush();
}

void PdfSignatureGenerator::GetSignedPdf(UUCByteArray& signedPdf)
{
	int finalLength = m_pSignOutputDevice->GetLength();
	char* szSignedPdf = new char[finalLength];
	
	m_pSignOutputDevice->Seek(0);
	int nRead = m_pSignOutputDevice->Read(szSignedPdf, finalLength);
	
	signedPdf.append((BYTE*)szSignedPdf, nRead);
	
	delete szSignedPdf;
}

const double PdfSignatureGenerator::getWidth(int pageIndex) {
	if (m_pPdfDocument) {
		PdfPage* pPage = m_pPdfDocument->GetPage(pageIndex);
		return pPage->GetPageSize().GetWidth();
	}
	return 0;
}

const double PdfSignatureGenerator::getHeight(int pageIndex) {
	if (m_pPdfDocument) {
		PdfPage* pPage = m_pPdfDocument->GetPage(pageIndex);
		return pPage->GetPageSize().GetHeight();
	}
	return 0;
}

const double PdfSignatureGenerator::lastSignatureY(int left, int bottom) {
	if(!m_pPdfDocument)
		return -1;
	/// Find the document catalog dictionary
	const PdfObject *const trailer = m_pPdfDocument->GetTrailer();
	if (! trailer->IsDictionary())
		return -1;
	const PdfObject *const catalogRef =	trailer->GetDictionary().GetKey(PdfName("Root"));
	if (catalogRef==0 || ! catalogRef->IsReference())
		return -2;//throw std::invalid_argument("Invalid /Root entry");
	const PdfObject *const catalog =
		m_pPdfDocument->GetObjects().GetObject(catalogRef->GetReference());
	if (catalog==0 || !catalog->IsDictionary())
		return -3;//throw std::invalid_argument("Invalid or non-dictionary
	//referenced by /Root entry");
	
	/// Find the Fields array in catalog dictionary
	const PdfObject *acroFormValue = catalog->GetDictionary().GetKey(PdfName("AcroForm"));
	if (acroFormValue == 0)
		return bottom;
	if (acroFormValue->IsReference())
		acroFormValue = m_pPdfDocument->GetObjects().GetObject(acroFormValue->GetReference());
	
	if (!acroFormValue->IsDictionary())
		return bottom;
	
	const PdfObject *fieldsValue = acroFormValue->GetDictionary().GetKey(PdfName("Fields"));
	if (fieldsValue == 0)
		return bottom;
	
	if (fieldsValue->IsReference())
		fieldsValue = m_pPdfDocument->GetObjects().GetObject(acroFormValue->GetReference());
	
	if (!fieldsValue->IsArray())
		return bottom;
	
	vector<const PdfObject*> signatureVector;
	
	/// Verify if each object of the array is a signature field
	const PdfArray &array = fieldsValue->GetArray();
	
	int minY = bottom;
	
	for (unsigned int i=0; i<array.size(); i++) {
		const PdfObject * pObj = m_pPdfDocument->GetObjects().GetObject(array[i].GetReference());
		if (IsSignatureField(m_pPdfDocument, pObj)) {
			const PdfObject *const keyRect = pObj->GetDictionary().GetKey(PdfName("Rect"));
			if (keyRect == 0) {
				return bottom;
			}
			PdfArray rectArray = keyRect->GetArray();
			PdfRect rect;
			rect.FromArray(rectArray);
			
			if (rect.GetLeft() == left) {
				minY = (rect.GetBottom() <= minY && rect.GetBottom()!=0) ? rect.GetBottom()-85 : minY;
			}
		}
	}
	return minY;
}

bool PdfSignatureGenerator::IsSignatureField(const PdfMemDocument* pDoc, const PdfObject *const pObj)
{
	if (pObj == 0) return false;
	
	if (!pObj->IsDictionary())
		return false;
	
	const PdfObject *const keyFTValue = pObj->GetDictionary().GetKey(PdfName("FT"));
	if (keyFTValue == 0)
		return false;
	
	string value;
	keyFTValue->ToString(value);
	if (value != "/Sig")
		return false;
	
	const PdfObject *const keyVValue = pObj->GetDictionary().GetKey(PdfName("V"));
	if (keyVValue == 0)
		return false;
	
	const PdfObject *const signature = pDoc->GetObjects().GetObject(keyVValue->GetReference());
	if (signature->IsDictionary())
		return true;
	else
		return false;
}
