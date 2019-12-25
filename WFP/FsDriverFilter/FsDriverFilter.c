/*++

Module Name:

    FsDriverFilter.c

Abstract:

    This is the main module of the FsDriverFilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>


PFLT_FILTER pFilterHandle = NULL;

FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA pData, PCFLT_RELATED_OBJECTS pFltObject, PVOID* pCompletionContext, FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA pData, PCFLT_RELATED_OBJECTS pFltObject, PVOID* pCompletionContext);

FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA pData, PCFLT_RELATED_OBJECTS pFltObject, PVOID* pCompletionContext);


NTSTATUS MiniUnload(
	FLT_FILTER_UNLOAD_FLAGS Flags
);


const FLT_OPERATION_REGISTRATION CallBacks[] = 
{
	{IRP_MJ_CREATE,0,MiniPreCreate,MiniPostCreate},
	{IRP_MJ_WRITE,0,MiniPreWrite,NULL},
	{IRP_MJ_OPERATION_END}
};


const FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	CallBacks,
	MiniUnload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA pData, PCFLT_RELATED_OBJECTS pFltObject, PVOID* pCompletionContext,FLT_POST_OPERATION_FLAGS Flags)
{
	KdPrint(("WFP: Post create is running \r\n"));
	return FLT_POSTOP_FINISHED_PROCESSING;
		
}

FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA pData,PCFLT_RELATED_OBJECTS pFltObject,PVOID * pCompletionContext)
{

	PFLT_FILE_NAME_INFORMATION pFileNameInfo = NULL;
	NTSTATUS status = 0;
	WCHAR NameFile[256] = { 0 };
	
	status = FltGetFileNameInformation(
		pData, 
		FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
		&pFileNameInfo);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = FltParseFileNameInformation(pFileNameInfo);

	if (!NT_SUCCESS(status))
	{
		goto clear;
	}

	if (pFileNameInfo->Name.MaximumLength > 256)
	{
		goto  clear;
	}


	RtlCopyMemory(NameFile, pFileNameInfo->Name.Buffer, pFileNameInfo->Name.MaximumLength);
	KdPrint(("WFP: Create file is: %ws \r\n ", NameFile));
	
	
	clear:

		if (pFileNameInfo != NULL)
		{
			FltReleaseFileNameInformation(pFileNameInfo);
		}
	
	


	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
	
}

FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA pData, PCFLT_RELATED_OBJECTS pFltObject, PVOID* pCompletionContext)
{

	PFLT_FILE_NAME_INFORMATION pFileNameInfo = NULL;
	NTSTATUS status = 0;
	WCHAR NameFile[256] = { 0 };

	status = FltGetFileNameInformation(
		pData,
		FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
		&pFileNameInfo);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = FltParseFileNameInformation(pFileNameInfo);

	if (!NT_SUCCESS(status))
	{
		goto clear;
	}

	if (pFileNameInfo->Name.MaximumLength > 256)
	{
		goto  clear;
	}


	RtlCopyMemory(NameFile, pFileNameInfo->Name.Buffer, pFileNameInfo->Name.MaximumLength);
	_wcsupr(NameFile); //to up case
	
	if (wcsstr(NameFile,L"OPENME.TXT") != NULL)
	{
		KdPrint(("WFP: File %ws blocked! \r\n", NameFile));
		pData->IoStatus.Status = STATUS_INVALID_PARAMETER;
		pData->IoStatus.Information = 0;
		FltReleaseFileNameInformation(pFileNameInfo);
		return FLT_PREOP_COMPLETE;
	}
	


clear:

	if (pFileNameInfo != NULL)
	{
		FltReleaseFileNameInformation(pFileNameInfo);
	}




	return FLT_PREOP_SUCCESS_NO_CALLBACK;

}


NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{

	UNREFERENCED_PARAMETER(Flags);
	
	KdPrint(("WFP driver unload!"));

	FltUnregisterFilter(pFilterHandle);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{

	UNREFERENCED_PARAMETER(pRegistryPath);
	
	NTSTATUS status = 0;
		
	status = FltRegisterFilter(
		pDriverObject,
		&FilterRegistration,
		&pFilterHandle
	);

	if (!NT_SUCCESS(status))
	{
		return status;
		
	}

	status = FltStartFiltering(pFilterHandle);
	if (!NT_SUCCESS(status))
	{
		FltUnregisterFilter(pFilterHandle);
	}
	


	return status;
}
