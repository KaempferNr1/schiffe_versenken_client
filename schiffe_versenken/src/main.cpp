#include <cmath>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <imgui-SFML.h>
#include <imgui.h>


#include "Utils/Soundsystem.h"
#include "Utils/Log.h"
#include "Utils/Random.h"
#include "Utils/Timer.h"

#include "Eventsystem.h"
#include "Layer.h"
#include "LayerManager.h"
#include "UI/Button.h"
#include "Menu.h"

static int Main(int argc, char** argv);

static [[nodiscard]] bool init_sfml_imgui(sf::RenderWindow& window);

static [[nodiscard]] std::shared_ptr<Eventsystem> init_eventsystem(sf::RenderWindow& window);

static [[nodiscard]] std::shared_ptr<LayerManager> init_layer_manager();

static [[nodiscard]] std::shared_ptr<Soundsystem> init_soundsystem();

static void save_volumes(const std::shared_ptr<Soundsystem>& soundsystem);


#if (defined(PLATFORM_WINDOWS) || defined (_WIN64)) && defined(DIST)
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    return Main(__argc, __argv);
}

#else

int main(const int argc, char** argv)
{
    return Main(argc, argv);
}

#endif



int Main([[maybe_unused]] const int argc, [[maybe_unused]] char** argv)
{
    Log::init(LOG_LEVEL_INFO,LOG_LEVEL_INFO);
    Random::init();

	sf::RenderWindow window(sf::VideoMode({ 720, 480 }), "schiffe_versenken_client", sf::Style::Default);
	if (!init_sfml_imgui(window))
    {
        LOG_ERROR("failed to initialize imgui");
    }

    LOG_INFO("OpenGL Info:");
    LOG_INFO("Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    LOG_INFO("Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    LOG_INFO("Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    std::shared_ptr<Eventsystem> eventsystem = init_eventsystem(window);
    std::shared_ptr<Soundsystem> soundsystem = init_soundsystem();
    std::shared_ptr<LayerManager> layer_manager = init_layer_manager();


	window.setFramerateLimit(0);
    sf::Clock delta_clock;
    double current_fps = 0;
    int current_frames = 0;
    double condt = 0;
	while (window.isOpen())
    {
        current_frames++;
        eventsystem->handle_updates(window);
		const double deltatime = static_cast<double>(delta_clock.getElapsedTime().asSeconds());
		ImGui::SFML::Update(window, delta_clock.restart());
        condt += deltatime;

        ImGui::Begin("debug");
        if (condt > 1)
        {
            current_fps = static_cast<double>(current_frames) / condt;
            condt = 0;
            current_frames = 0;
        }

		ImGui::Text("fps: %.2f", current_fps);

		ImGui::End();
    	const std::shared_ptr<Layer> current_layer = layer_manager->get_top();
		current_layer->update(eventsystem,layer_manager,soundsystem,window,deltatime);

		soundsystem->update();

		window.clear();
		current_layer->render(window);
		ImGui::SFML::Render(window);
		window.display();

        if (layer_manager->is_empty())
        {
            window.close();
        }
    }

    ImGui::SFML::Shutdown(window);

    save_volumes(soundsystem);

	return 0;
}



#include "../Resources/Images/Roboto-Regular.embed"
bool init_sfml_imgui(sf::RenderWindow& window)
{

    if (!ImGui::SFML::Init(window, false))
        return false;
    
    ImGui::SFML::ProcessEvent(window, sf::Event::FocusLost());
    ImGui::SFML::ProcessEvent(window, sf::Event::FocusGained());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows // funktioniert nicht weil die sfml backends das nicht haben

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    ImFont* roboto_font = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(g_RobotoRegular), sizeof g_RobotoRegular, 20.0f, &font_config);
    io.FontDefault = roboto_font;
    if (!ImGui::SFML::UpdateFontTexture()) 
    {
        LOG_ERROR("failed to update font");
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    return true;
}


std::shared_ptr<Eventsystem> init_eventsystem(sf::RenderWindow& window)
{
    auto eventsystem = std::make_shared<Eventsystem>(window);
    eventsystem->add_key_listener(sf::Keyboard::Key::W); 
    eventsystem->add_key_listener(sf::Keyboard::Key::A);
    eventsystem->add_key_listener(sf::Keyboard::Key::S);
    eventsystem->add_key_listener(sf::Keyboard::Key::D);
    eventsystem->add_key_listener(sf::Keyboard::Key::G);
    eventsystem->add_key_listener(sf::Keyboard::Key::Q);
    eventsystem->add_key_listener(sf::Keyboard::Key::R);
    eventsystem->add_key_listener(sf::Keyboard::Key::T);
    eventsystem->add_key_listener(sf::Keyboard::Key::E);
    eventsystem->add_key_listener(sf::Keyboard::Key::Up);
    eventsystem->add_key_listener(sf::Keyboard::Key::Down);
    eventsystem->add_key_listener(sf::Keyboard::Key::Left);
    eventsystem->add_key_listener(sf::Keyboard::Key::Right);
    eventsystem->add_key_listener(sf::Keyboard::Key::Enter);
    eventsystem->add_key_listener(sf::Keyboard::Key::Escape);
    eventsystem->add_key_listener(sf::Keyboard::Key::LShift);

    eventsystem->add_mouse_button_listener(sf::Mouse::Button::Left);

    return eventsystem;
}

std::shared_ptr<LayerManager> init_layer_manager()
{
    auto layer_manager = std::make_shared<LayerManager>();
    layer_manager->push_layer(std::make_shared<Menu>());
    return layer_manager;
}


std::shared_ptr<Soundsystem> init_soundsystem()
{
    auto soundsystem = std::make_shared<Soundsystem>(0.f, false);
    //TODO: ADD ADITIONAL SOUNDS
    soundsystem->add_group("ui_sounds");
    soundsystem->add_group("game_sounds");

    const std::filesystem::path music_sounds{ std::filesystem::path("Resources") / "Sounds" / "Music" };
    std::vector<int> music_indices;
    for (auto const& dir_entry : std::filesystem::directory_iterator{ music_sounds })
    {
        LOG_INFO("{}", dir_entry.path().string());
        music_indices.emplace_back(static_cast<int>(music_indices.size()));
        soundsystem->load_buffer(dir_entry.path().string(), false, "music");
    }
    soundsystem->set_music_indices(music_indices);
    soundsystem->set_should_play_music(!music_indices.empty());


    std::ifstream fin("optionen.txt");
    std::unordered_map volumes = soundsystem->get_volumes();
    
	if (fin.is_open())
    {
	    while (!fin.eof())
	    {
		    try
		    {
                std::string group;
                std::getline(fin, group, ';');
                if (group.empty())
                    continue;
                std::string line;
                float volume;
                std::getline(fin,line);
                volume = std::stof(line);
                volumes[group] = volume;
		    }
		    catch (...)
		    {
                continue;
		    }


	    }
    }
    else
    {
        std::ofstream fout("optionen.txt");
        fout.close();
    }
    soundsystem->set_volume(volumes);

    return soundsystem;
}

void save_volumes(const std::shared_ptr<Soundsystem>& soundsystem)
{
    std::unordered_map volumes = soundsystem->get_volumes();
    std::ofstream fout("optionen.txt");
    if(!fout.is_open())
    {
        LOG_ERROR("couldnt open/create optionen.txt");
        return;
    }

    for (const auto& [group,volume] : volumes)
    {
        fout << group << ';' << volume << '\n';
    }
}
