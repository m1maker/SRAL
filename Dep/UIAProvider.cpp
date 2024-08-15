/*************************************************************************************************
 * Description: Implementation of the Provider class, which implements IRawElementProviderSimple 
 * and IInvokeProvider for a simple custom control.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 *************************************************************************************************/

#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <ole2.h>
#include <UIAutomation.h>

#include "Provider.h"

Provider::Provider(HWND hwnd): m_refCount(1), m_controlHWnd(hwnd)
{
    // Nothing to do.
}

Provider::~Provider()
{
    // Nothing to do.
}

// IUnknown implementation.

IFACEMETHODIMP_(ULONG) Provider::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) Provider::Release()
{
    long val = InterlockedDecrement(&m_refCount);
    if (val == 0)
    {
        delete this;
    }
    return val;
}

IFACEMETHODIMP Provider::QueryInterface(REFIID riid, void** ppInterface)
{
    if (riid == __uuidof(IUnknown))
    {
        *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    }
    else if (riid == __uuidof(IRawElementProviderSimple))
    {
        *ppInterface =static_cast<IRawElementProviderSimple*>(this);
    }
    else if (riid == __uuidof(IInvokeProvider))
    {
        *ppInterface =static_cast<IInvokeProvider*>(this);
    }
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    (static_cast<IUnknown*>(*ppInterface))->AddRef();
    return S_OK;
}


// IRawElementProviderSimple implementation

// Get provider options.

IFACEMETHODIMP Provider::get_ProviderOptions( ProviderOptions* pRetVal )
{
    *pRetVal = ProviderOptions_ServerSideProvider;
    return S_OK;
}

// Get the object that supports IInvokePattern.

IFACEMETHODIMP Provider::GetPatternProvider(PATTERNID patternId, IUnknown** pRetVal)
{
    if (patternId == UIA_InvokePatternId) 
    {
        AddRef();
        *pRetVal = static_cast<IRawElementProviderSimple*>(this);
    }
    else
    {
        *pRetVal = NULL;   
    }
    return S_OK;
}

// Gets custom properties.

IFACEMETHODIMP Provider::GetPropertyValue(PROPERTYID propertyId, VARIANT* pRetVal)
{
    if (propertyId == UIA_ControlTypePropertyId)            
    {
        pRetVal->vt = VT_I4;
        pRetVal->lVal = UIA_ButtonControlTypeId;            
    }

    // The Name property comes from the Caption property of the control window, if it has one.
    // The Name is overridden here for the sake of illustration. 
    else if (propertyId == UIA_NamePropertyId) 
    {
        pRetVal->vt = VT_BSTR;
        pRetVal->bstrVal = SysAllocString(L"ColorButton");
    }
    else
    {
        pRetVal->vt = VT_EMPTY;
       // UI Automation will attempt to get the property from the host window provider.
    }
    return S_OK;
}

// Gets the UI Automation provider for the host window. This provider supplies most properties.

IFACEMETHODIMP Provider::get_HostRawElementProvider(IRawElementProviderSimple** pRetVal)
{
    return UiaHostProviderFromHwnd(m_controlHWnd, pRetVal); 
}


// IInvokeProvider implementation.

IFACEMETHODIMP Provider::Invoke()
{
    PostMessage(m_controlHWnd,  WM_LBUTTONDOWN, NULL, NULL);
    return S_OK;
}
