#include <stdlib.h>
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
    #define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <imgui_internal.h>
#include <ImNodesEz.h>

#include <cstdlib>
#include <iostream>

#include <map>

/// A structure defining a connection between two slots of two nodes.
struct Connection
{
    /// `id` that was passed to BeginNode() of input node.
    void* InputNode = nullptr;
    /// Descriptor of input slot.
    const char* InputSlot = nullptr;
    /// `id` that was passed to BeginNode() of output node.
    void* OutputNode = nullptr;
    /// Descriptor of output slot.
    const char* OutputSlot = nullptr;

    bool operator==(const Connection& other) const
    {
        return InputNode == other.InputNode &&
               InputSlot == other.InputSlot &&
               OutputNode == other.OutputNode &&
               OutputSlot == other.OutputSlot;
    }

    bool operator!=(const Connection& other) const
    {
        return !operator ==(other);
    }
};

enum NodeSlotTypes
{
    NodeSlotPosition = 1,   // ID can not be 0
    NodeSlotRotation,
    NodeSlotMatrix,
};

/// A structure holding node state.
struct MyNode
{
    /// Title which will be displayed at the center-top of the node.
    const char* Title = nullptr;
    /// Flag indicating that node is selected by the user.
    bool Selected = false;
    /// Node position on the canvas.
    ImVec2 Pos{};
    /// List of node connections.
    std::vector<Connection> Connections{};
    /// A list of input slots current node has.
    std::vector<ImNodes::Ez::SlotInfo> InputSlots{};
    /// A list of output slots current node has.
    std::vector<ImNodes::Ez::SlotInfo> OutputSlots{};

    explicit MyNode(const char* title,
        const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
        const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
    {
        Title = title;
        InputSlots = input_slots;
        OutputSlots = output_slots;
    }

    /// Deletes connection from this node.
    void DeleteConnection(const Connection& connection)
    {
        for (auto it = Connections.begin(); it != Connections.end(); ++it)
        {
            if (connection == *it)
            {
                Connections.erase(it);
                break;
            }
        }
    }
};

std::map<std::string, MyNode*(*)()> available_nodes{
    {"Compose", []() -> MyNode* { return new MyNode("Compose", {
        {"Position", NodeSlotPosition}, {"Rotation", NodeSlotRotation}  // Input slots
    }, {
        {"Matrix", NodeSlotMatrix}                                      // Output slots
    }); }},
    {"Decompose", []() -> MyNode* { return new MyNode("Decompose", {
        {"Matrix", NodeSlotMatrix}                                      // Input slots
    }, {
        {"Position", NodeSlotPosition}, {"Rotation", NodeSlotRotation}  // Output slots
    }); }},
};
std::vector<MyNode*> nodes;

void drawImGui() {
    // Canvas must be created after ImGui initializes, because constructor accesses ImGui style to configure default colors.
    static ImNodes::Ez::Context* context = ImNodes::Ez::CreateContext();
    IM_UNUSED(context);

    if (ImGui::Begin("ImNodes", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        // We probably need to keep some state, like positions of nodes/slots for rendering connections.
        ImNodes::Ez::BeginCanvas();
        for (auto it = nodes.begin(); it != nodes.end();)
        {
            MyNode* node = *it;

            // Start rendering node
            if (ImNodes::Ez::BeginNode(node, node->Title, &node->Pos, &node->Selected))
            {
                // Render input nodes first (order is important)
                ImNodes::Ez::InputSlots(node->InputSlots.data(), node->InputSlots.size());

                // Custom node content may go here
                ImGui::Text("Content of %s", node->Title);

                // Render output nodes first (order is important)
                ImNodes::Ez::OutputSlots(node->OutputSlots.data(), node->OutputSlots.size());

                // Store new connections when they are created
                Connection new_connection;
                if (ImNodes::GetNewConnection(&new_connection.InputNode, &new_connection.InputSlot,
                                              &new_connection.OutputNode, &new_connection.OutputSlot))
                {
                    ((MyNode*) new_connection.InputNode)->Connections.push_back(new_connection);
                    ((MyNode*) new_connection.OutputNode)->Connections.push_back(new_connection);
                }

                // Render output connections of this node
                for (const Connection& connection : node->Connections)
                {
                    // Node contains all it's connections (both from output and to input slots). This means that multiple
                    // nodes will have same connection. We render only output connections and ensure that each connection
                    // will be rendered once.
                    if (connection.OutputNode != node)
                        continue;

                    if (!ImNodes::Connection(connection.InputNode, connection.InputSlot, connection.OutputNode,
                                             connection.OutputSlot))
                    {
                        // Remove deleted connections
                        ((MyNode*) connection.InputNode)->DeleteConnection(connection);
                        ((MyNode*) connection.OutputNode)->DeleteConnection(connection);
                    }
                }
            }
            // Node rendering is done. This call will render node background based on size of content inside node.
            ImNodes::Ez::EndNode();

            if (node->Selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete) && ImGui::IsWindowFocused())
            {
                // Deletion order is critical: first we delete connections to us
                for (auto& connection : node->Connections)
                {
                    if (connection.OutputNode == node)
                    {
                        ((MyNode*) connection.InputNode)->DeleteConnection(connection);
                    }
                    else
                    {
                        ((MyNode*) connection.OutputNode)->DeleteConnection(connection);
                    }
                }
                // Then we delete our own connections, so we don't corrupt the list
                node->Connections.clear();
                
                delete node;
                it = nodes.erase(it);
            }
            else
                ++it;
        }

        if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
        {
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::OpenPopup("NodesContextMenu");
        }

        if (ImGui::BeginPopup("NodesContextMenu"))
        {
            for (const auto& desc : available_nodes)
            {
                if (ImGui::MenuItem(desc.first.c_str()))
                {
                    nodes.push_back(desc.second());
                    ImNodes::AutoPositionNode(nodes.back());
                }
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Reset Zoom"))
                ImNodes::GetCurrentCanvas()->Zoom = 1;

            if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImNodes::Ez::EndCanvas();
    }
    ImGui::End();
}

int main() {
    if(!glfwInit()) {
        std::cerr << "Failed to init" << std::endl;
        return EXIT_FAILURE;
    }

    std::atexit(glfwTerminate);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImNodes", nullptr, nullptr);
    if(window == nullptr) {
        std::cerr << "Failed to create window" << std::endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    while(!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawImGui();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return EXIT_SUCCESS;
}
