#include "DisplayHandler.h"
#include "TransferHandler.h"

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName(Version::PROJECT);
	v.AuthorName("JiuZhouCangLan");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = (Version::PROJECT).data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver
#	ifndef SKYRIMVR
		< SKSE::RUNTIME_1_5_39
#	else
		> SKSE::RUNTIME_VR_1_4_15_1
#	endif
	) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	if (std::filesystem::exists("Data/SKSE/Plugins/TCCDisplayHelper.debugLog")) {
		log->set_level(spdlog::level::debug);
		log->flush_on(spdlog::level::debug);
		log->info("debug mode");
	} else {
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
		log->info("info mode");
	}

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S] [%l] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

namespace Papyrus
{
	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		Functions::DisplayHandler::Bind(*a_vm);
		Functions::TransferHandler::Bind(*a_vm);
		return true;
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	SKSE::Init(a_skse, false);

	SKSE::AllocTrampoline(256);

	const auto papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(Papyrus::Bind);

	return true;
}

extern "C" DLLEXPORT const char* APIENTRY GetPluginVersion()
{
	return Version::NAME.data();
}
