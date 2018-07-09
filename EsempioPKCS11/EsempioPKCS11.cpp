// EsempioPKCS11.cpp : Defines the entry point for the console application.
//
#include <functional>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#pragma pack(1)
#include "pkcs11.h"
#pragma pack()

using namespace std;
typedef unsigned char BYTE;

// la specifica PKCS#11 di riferimento pu� essere consultata all'indirizzo https://www.cryptsoft.com/pkcs11doc/STANDARD/pkcs-11v2-11r1.pdf


int main(int argc, char* argv[])
{
	// carica la libreria del middleware
	void* mod = dlopen("libciepki.so", RTLD_LAZY);

	if (!mod) {
		fprintf(stderr, "%s\n", dlerror());
	        exit(1);
	}

	// ottiene il puntatore alla funzione C_GetFunctionList, da cui si ottengono tutte le altre funzioni del PKCS11
	CK_C_GetFunctionList GetFunctionList = (CK_C_GetFunctionList)dlsym(mod, "C_GetFunctionList");
	CK_FUNCTION_LIST_PTR FunctionList;
	GetFunctionList(&FunctionList);
	FunctionList->C_Initialize(NULL);

	CK_SLOT_ID Slots[20];
	CK_SLOT_ID Slot;
	CK_ULONG SlotsNum = 20;
	// chiedo la lista degli slot (lettori di smart card) con un smart card connessa
	FunctionList->C_GetSlotList(TRUE, Slots, &SlotsNum);

	// se non c'� nessun lettore con una smart card connessa, chiedo di appoggiarne una e attendo un evento su uno slot
	if (SlotsNum == 0) {
		std::cout << "Appoggiare la CIE sul lettore\n";
		FunctionList->C_WaitForSlotEvent(0, &Slot, NULL);
	}
	else {
		Slot = Slots[0];
	}

	CK_SESSION_HANDLE Session;
	// apro una sessione con lo slot
	CK_RV result = FunctionList->C_OpenSession(Slot, CKF_SERIAL_SESSION, NULL, NULL, &Session);

	// se la CIE non � abilitata o viene allontanata durante la lettura si genera un errore
	if (result != CKR_OK) {
		std::cout << "Errore nella lettura della CIE\n";
		dlclose(mod);
		exit(2);
	}

	// chiedo le informazioni sul token (la CIE) e ricavo il serialNumber, che corrisponde all'idServizi
	CK_TOKEN_INFO Token;
	FunctionList->C_GetTokenInfo(Slot, &Token);
	std::cout << "ID_Servizi: " << std::string((char*)Token.serialNumber, sizeof(Token.serialNumber)) << "\n";

	// cerco il certificato dell'utente tramite un template che contiene solo la classe (CKO_CERTIFICATE)
	CK_ATTRIBUTE CertAttribute;
	CK_OBJECT_CLASS CertClass = CKO_CERTIFICATE;
	CertAttribute.type = CKA_CLASS;
	CertAttribute.pValue = &CertClass;
	CertAttribute.ulValueLen = sizeof(CertClass);
	FunctionList->C_FindObjectsInit(Session, &CertAttribute, 1);
	CK_OBJECT_HANDLE Object;
	CK_ULONG NumObjects=0;
	FunctionList->C_FindObjects(Session, &Object, 1, &NumObjects);
	FunctionList->C_FindObjectsFinal(Session);

	CK_ATTRIBUTE CertValue;
	CertValue.type = CKA_VALUE;
	CertValue.pValue = NULL;
	CertValue.ulValueLen = 0;

	// leggo il valore del certificato
	FunctionList->C_GetAttributeValue(Session, Object, &CertValue, 1);
	std::vector<BYTE> CertBuffer(CertValue.ulValueLen);
	CertValue.pValue = CertBuffer.data();
	FunctionList->C_GetAttributeValue(Session, Object, &CertValue, 1);

	// chiude la sessione e termina il PKCS11
	FunctionList->C_CloseSession(Session);
	FunctionList->C_Finalize(NULL);

	// crea l'oggetto di sistema del certificato
	const BYTE *p = (BYTE*)CertValue.pValue;
	unique_ptr<X509,std::function<void(X509*)>> cer{d2i_X509(nullptr, &p, CertValue.ulValueLen), [](X509* val){ X509_free(val);}};
	if (cer.get() == nullptr) {
		std::cout << "Certificato DS non valido\n";
		dlclose(mod);
		exit(3);
	}

	// converte il Subject name in stringa e lo scrive sullo schermo
	char *subj = X509_NAME_oneline(X509_get_subject_name(cer.get()), NULL, 0);
	//char *issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
	std::cout << "Titolare :" << subj << "\n";
	OPENSSL_free(subj);

	dlclose(mod);

	return 0;
}
