static bool loadSkip{ false };
static bool loadMenu{ false };

class EventHandler :
    public REX::Singleton<EventHandler>,
    public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    RE::EventResult ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (const auto manager = RE::BGSSaveLoadManager::GetSingleton()) {
            if (manager->saveGameListBuilt && !manager->saveGameCount)
                loadSkip = true;
        }

        if (loadSkip) {
            RE::UI::GetSingleton()->UnregisterSink<RE::MenuOpenCloseEvent>(this);
            return RE::EventResult::kContinue;
        }

        if (a_event.menuName == "FaderMenu" && !a_event.opening) {
            RE::UI::GetSingleton()->UnregisterSink<RE::MenuOpenCloseEvent>(this);

            const auto queue = RE::UIMessageQueue::GetSingleton();
            queue->AddMessage("LoadingMenu", RE::UI_MESSAGE_TYPE::kShow);

            loadMenu = true;
        }

        return RE::EventResult::kContinue;
    }

    static void Init()
    {
        RE::UI::GetSingleton()->RegisterSink<RE::MenuOpenCloseEvent>(GetSingleton());
    }
};

void OnMessage(SFSE::MessagingInterface::Message* a_msg)
{
    if (a_msg->type == SFSE::MessagingInterface::kPostPostLoad)
        EventHandler::Init();

    if (a_msg->type == SFSE::MessagingInterface::kPostPostDataLoad && !loadSkip) {
        const auto manager = RE::BGSSaveLoadManager::GetSingleton();
        manager->QueueLoadMostRecentSaveGame();
    }
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
    SFSE::Init(a_sfse);

    if (const auto message = SFSE::GetMessagingInterface())
        message->RegisterListener(OnMessage);

    if (const auto task = SFSE::GetTaskInterface()) {
        task->AddPermanentTask([] {
            if (!loadSkip && (REX::W32::GetKeyState(REX::W32::VK_SPACE) & 0x8000) != 0)
                loadSkip = true;
            
            if (loadMenu && loadSkip) {
                loadMenu = false;
                const auto queue = RE::UIMessageQueue::GetSingleton();
                queue->AddMessage("LoadingMenu", RE::UI_MESSAGE_TYPE::kHide);
            }
        });
    }

    return true;
}
