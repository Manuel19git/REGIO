#pragma once

#define GFX_EXCEPT_NOINFO(hr) HrException( __LINE__,__FILE__,(hr) );
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw HrException( __LINE__,__FILE__,hr );
#define PROFILE_SCOPE() ProfilerTimer timer(__FUNCSIG__)

#ifndef NDEBUG
#define GFX_EXCEPT(hr) HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw InfoException( __LINE__,__FILE__,v );}}
#define GFX_DEVICE_REMOVED_EXCEPT(hr) DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_THROW_INFO_ONLY(call) (call)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) DeviceRemovedException( __LINE__,__FILE__,(hr) )
#endif

