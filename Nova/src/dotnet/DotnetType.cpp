//#include <Nova/dotnet/DotnetType.hpp>
//#include <Nova/dotnet/Host.hpp>
//#include <Nova/debug/Profile.hpp>
//#include <Nova/debug/Log.hpp>
//#include <stdexcept>
//#include <span>
//
//using namespace Nova;
//
//void DotnetType::AddInternalCallImmediate(const std::string_view name, const void *functionPtr) const
//{
//	NV_PROFILE_FUNC;
//
//	InternalCallUpload upload{name.data(), functionPtr};
//	Dotnet::UploadInternalCalls_(m_ID, std::span(&upload, 1));
//
//	NV_LOG_TRACE("Uploaded internal call for .NET type {}: {} (function ptr: {}).", m_FullName, name, functionPtr);
//}
//
//void DotnetType::UploadInternalCalls()
//{
//	NV_PROFILE_FUNC;
//	
//	Dotnet::UploadInternalCalls_(m_ID, m_InternalCallUploadCache);
//
//	NV_LOG_TRACE("Uploaded {} internal calls for .NET type {}.", m_InternalCallUploadCache.size(), m_FullName);
//
//	m_InternalCallUploadCache.clear();
//}