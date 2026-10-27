#include "template.h"

#include <cstdio>

namespace
{
CConVar<int> template_enabled("template_enabled", FCVAR_NONE, "Enable or disable the plugin's example gameplay logic.", 1, true, 0.0f, true,
							 1.0f);

void SetError(char* error, size_t maxlen, const char* message)
{
	if (error && maxlen > 0)
	{
		std::snprintf(error, maxlen, "%s", message);
	}
}

KHook::Return<void> HookGameFrame(ISource2Server*, bool simulating, bool, bool)
{
	g_Template.OnGameFrame(simulating);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientPutInServer(ISource2GameClients*, CPlayerSlot slot, const char* name, int type, uint64 xuid)
{
	g_Template.OnClientPutInServer(slot, name, type, xuid);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientActive(ISource2GameClients*, CPlayerSlot slot, bool loadGame, const char* name, uint64 xuid)
{
	g_Template.OnClientActive(slot, loadGame, name, xuid);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientFullyConnect(ISource2GameClients*, CPlayerSlot slot)
{
	g_Template.OnClientFullyConnect(slot);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientDisconnect(ISource2GameClients*, CPlayerSlot slot, ENetworkDisconnectionReason, const char* name, uint64 xuid,
										 const char*)
{
	g_Template.OnClientDisconnect(slot, name, xuid);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientCommand(ISource2GameClients*, CPlayerSlot slot, const CCommand& command)
{
	g_Template.OnClientCommand(slot, command);
	return {KHook::Action::Ignore};
}

KHook::Return<void> HookClientSettingsChanged(ISource2GameClients*, CPlayerSlot slot)
{
	g_Template.OnClientSettingsChanged(slot);
	return {KHook::Action::Ignore};
}

KHook::Virtual<ISource2Server, void, bool, bool, bool> gameFrameHook{&ISource2Server::GameFrame, nullptr, &HookGameFrame};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot, char const*, int, uint64> clientPutInServerHook{
	&ISource2GameClients::ClientPutInServer, nullptr, &HookClientPutInServer};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot, bool, const char*, uint64> clientActiveHook{
	&ISource2GameClients::ClientActive, nullptr, &HookClientActive};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot> clientFullyConnectHook{&ISource2GameClients::ClientFullyConnect, nullptr,
																				&HookClientFullyConnect};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot, ENetworkDisconnectionReason, const char*, uint64, const char*>
	clientDisconnectHook{&ISource2GameClients::ClientDisconnect, nullptr, &HookClientDisconnect};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot, const CCommand&> clientCommandHook{&ISource2GameClients::ClientCommand, nullptr,
																						 &HookClientCommand};
KHook::Virtual<ISource2GameClients, void, CPlayerSlot> clientSettingsChangedHook{&ISource2GameClients::ClientSettingsChanged, nullptr,
																				  &HookClientSettingsChanged};
} // namespace

Template g_Template;

PLUGIN_EXPOSE(Template, g_Template);

CON_COMMAND_F(template_status, "Show the template plugin status.", FCVAR_NONE)
{
	(void)context;
	(void)args;
	META_CONPRINTF("[%s] %s; example gameplay logic %s.\n", PLUGIN_LOGTAG, g_Template.IsLoaded() ? "running" : "not running",
				   template_enabled.Get() ? "enabled" : "disabled");
}

bool Template::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	(void)late;
	PLUGIN_SAVEVARS();

	if (!ismm)
	{
		SetError(error, maxlen, "Metamod API is unavailable.");
		return false;
	}

	server = static_cast<ISource2Server*>(ismm->VInterfaceMatch(ismm->GetServerFactory(), INTERFACEVERSION_SERVERGAMEDLL));
	gameClients = static_cast<ISource2GameClients*>(ismm->VInterfaceMatch(ismm->GetServerFactory(), INTERFACEVERSION_SERVERGAMECLIENTS));

	META_CONVAR_REGISTER(FCVAR_RELEASE | FCVAR_GAMEDLL);
	ismm->AddListener(this, this);
	loaded = true;
	InstallHooks();

	META_CONPRINTF("[%s] %s %s loaded.\n", PLUGIN_LOGTAG, PLUGIN_DISPLAY_NAME, PLUGIN_FULL_VERSION);
	return true;
}

void Template::AllPluginsLoaded()
{
	if (loaded)
	{
		InstallHooks();
		META_CONPRINTF("[%s] All Metamod plugins are loaded.\n", PLUGIN_LOGTAG);
	}
}

bool Template::QueryRunning(char* error, size_t maxlen)
{
	if (loaded)
	{
		return true;
	}

	SetError(error, maxlen, "The plugin has not completed loading.");
	return false;
}

bool Template::Unload(char* error, size_t maxlen)
{
	(void)error;
	(void)maxlen;

	RemoveHooks();
	loaded = false;
	paused = false;
	players.clear();
	server = nullptr;
	gameClients = nullptr;
	return true;
}

bool Template::Pause(char* error, size_t maxlen)
{
	(void)error;
	(void)maxlen;

	paused = true;
	RemoveHooks();
	return true;
}

bool Template::Unpause(char* error, size_t maxlen)
{
	paused = false;
	InstallHooks();
	return true;
}

void Template::OnLevelInit(char const* mapName, char const* mapEntities, char const* oldLevel, char const* landmarkName, bool loadGame,
						   bool background)
{
	(void)mapEntities;
	(void)oldLevel;
	(void)landmarkName;
	(void)loadGame;
	(void)background;

	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Map started: %s.\n", PLUGIN_LOGTAG, mapName ? mapName : "<unknown>");
	}
}

void Template::OnLevelShutdown()
{
	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Map is shutting down.\n", PLUGIN_LOGTAG);
	}
}

void Template::OnGameFrame(bool simulating)
{
	if (!loaded || paused || !template_enabled.Get() || !simulating)
	{
		return;
	}
}

void Template::OnClientPutInServer(CPlayerSlot slot, const char* name, int type, uint64 xuid)
{
	(void)type;
	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Client entered the server: slot %d, name %s.\n", PLUGIN_LOGTAG, slot.Get(), name ? name : "<unknown>");
	}
	auto& player = players[slot.Get()];
	player.slot = slot.Get();
	player.name = name ? name : "";
	player.steamId = xuid;
	player.inServer = true;
}

void Template::OnClientActive(CPlayerSlot slot, bool loadGame, const char* name, uint64 xuid)
{
	(void)loadGame;
	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Client is active: slot %d, name %s.\n", PLUGIN_LOGTAG, slot.Get(), name ? name : "<unknown>");
	}
	auto& player = players[slot.Get()];
	player.slot = slot.Get();
	player.name = name ? name : "";
	player.steamId = xuid;
	player.active = true;
}

void Template::OnClientFullyConnect(CPlayerSlot slot)
{
	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Client fully connected: slot %d.\n", PLUGIN_LOGTAG, slot.Get());
	}
	players[slot.Get()].fullyConnected = true;
}

void Template::OnClientDisconnect(CPlayerSlot slot, const char* name, uint64 xuid)
{
	(void)xuid;
	if (loaded && !paused)
	{
		META_CONPRINTF("[%s] Client disconnected: slot %d, name %s.\n", PLUGIN_LOGTAG, slot.Get(), name ? name : "<unknown>");
	}
	players.erase(slot.Get());
}

void Template::OnClientCommand(CPlayerSlot slot, const CCommand& command)
{
	(void)slot;
	(void)command;
}

void Template::OnClientSettingsChanged(CPlayerSlot slot)
{
	(void)slot;
}

const Template::PlayerInfo* Template::FindPlayer(CPlayerSlot slot) const
{
	const auto player = players.find(slot.Get());
	return player == players.end() ? nullptr : &player->second;
}

void Template::InstallHooks()
{
	if (!KHook::__exported__khook)
	{
		META_CONPRINTF("[%s] Metamod's hook service is unavailable; gameplay hooks are disabled.\n", PLUGIN_LOGTAG);
		return;
	}

	if (g_SMAPI && !server)
	{
		server = static_cast<ISource2Server*>(g_SMAPI->VInterfaceMatch(g_SMAPI->GetServerFactory(), INTERFACEVERSION_SERVERGAMEDLL));
	}
	if (g_SMAPI && !gameClients)
	{
		gameClients = static_cast<ISource2GameClients*>(
			g_SMAPI->VInterfaceMatch(g_SMAPI->GetServerFactory(), INTERFACEVERSION_SERVERGAMECLIENTS));
	}

	if (server && !serverHookInstalled)
	{
		gameFrameHook.Add(server);
		serverHookInstalled = true;
	}
	if (gameClients && !clientHooksInstalled)
	{
		clientPutInServerHook.Add(gameClients);
		clientActiveHook.Add(gameClients);
		clientFullyConnectHook.Add(gameClients);
		clientDisconnectHook.Add(gameClients);
		clientCommandHook.Add(gameClients);
		clientSettingsChangedHook.Add(gameClients);
		clientHooksInstalled = true;
	}

	if (!serverHookInstalled && !clientHooksInstalled)
	{
		META_CONPRINTF("[%s] Server hook interfaces are unavailable; lifecycle callbacks remain active.\n", PLUGIN_LOGTAG);
	}
}

void Template::RemoveHooks()
{
	if (server && serverHookInstalled)
	{
		gameFrameHook.Remove(server);
		serverHookInstalled = false;
	}
	if (gameClients && clientHooksInstalled)
	{
		clientPutInServerHook.Remove(gameClients);
		clientActiveHook.Remove(gameClients);
		clientFullyConnectHook.Remove(gameClients);
		clientDisconnectHook.Remove(gameClients);
		clientCommandHook.Remove(gameClients);
		clientSettingsChangedHook.Remove(gameClients);
		clientHooksInstalled = false;
	}
}
