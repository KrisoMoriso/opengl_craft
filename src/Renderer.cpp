#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"

#include "MeshSSBO.h"
#include "stb_image.h"

#include <format>
#include <map>

#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Game.h"
#include "ResourceManager.h"

Renderer::Renderer(World& worldInstance, ThreadPool& threadPoolInstance, int renderDistance) : m_world(worldInstance), m_threadPool(threadPoolInstance){
    m_renderDistance = renderDistance;
}

void Renderer::init(){
    m_shader = Shader("../src/shaders/voxel.vs.glsl", "../src/shaders/voxel.fs.glsl");
    loadTextures();
    setupBuffers();
}

// void Renderer::render(glm::mat4 viewMatrix){
//
//     glm::mat4 model(1.0f);
//     model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));
//
//     auto view = viewMatrix;
//
//     glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
//
//     glm::mat4 mvp = projection * view * model;
//
//     m_mesh.draw(mvp, m_shader.m_shaderProgramID, m_texture);
// }

void Renderer::loadTextures(){
    glUseProgram(m_shader.m_shaderProgramID);
    std::vector<std::string> textureFiles = {
        "../textures/block/dirt.png", //0
        "../textures/block/grass_block_side.png",
        "../textures/block/grass_block_top.png",
        "../textures/block/stone.png",
        "../textures/block/oak_planks.png",
        "../textures/block/oak_log.png",
        "../textures/block/oak_log_top.png",
        "../textures/block/sand.png",
        "../textures/block/water_still.png", //8
        "../textures/block/grass_block_side_overlay.png" //9
    };
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -1.5f);
    int width = 16, height = 16;
    int layerCount = textureFiles.size();
    int mipLevels = 5;
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, width, height, layerCount);
    for (int i = 0; i < layerCount; i++) {
        int widthImage, heightImage, nrChannels;
        unsigned char* data = stbi_load(textureFiles[i].c_str(), &widthImage, &heightImage, &nrChannels, 4);
        if (data != nullptr) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            printf("Failed to load texture: %s\n", textureFiles[i].c_str());
        }
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glUniform1i(glGetUniformLocation(m_shader.m_shaderProgramID, "texture0"), 0);


    int widthImage, heightImage, nrChannels;
    unsigned char* grassColorMapData = stbi_load("../textures/colormap/grass.png", &widthImage, &heightImage, &nrChannels, 4);
    glGenTextures(1, &m_grassColormap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_grassColormap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(glGetUniformLocation(m_shader.m_shaderProgramID, "grassColormap"), 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, grassColorMapData);
    if (grassColorMapData == nullptr) printf("failed to load grass colormap");
    stbi_image_free(grassColorMapData);

}



void Renderer::setupBuffers(){
    glGenVertexArrays(1, &m_emptyVAO);
    glBindVertexArray(m_emptyVAO);
}


void Renderer::update_meshes(glm::vec3 player_pos){
    MeshResult finished_mesh;
    while (m_result_queue_priority.try_pop(finished_mesh)){
        upload_mesh_to_gpu(finished_mesh);
    }
    while (m_result_queue.try_pop(finished_mesh)){
        upload_mesh_to_gpu(finished_mesh);
    }

    World::ChunkPos chunk_pos = World::get_chunk_position(player_pos);
    int render_distance = m_renderDistance;
    bool needs_sorting = false;


    for (int x = chunk_pos.x - render_distance; x <= chunk_pos.x + render_distance; ++x) {
        for (int z = chunk_pos.z - render_distance; z <= chunk_pos.z + render_distance; ++z) {
            for (int y = 0; y < World::WORLD_CHUNK_HEIGHT; ++y) {

                World::ChunkPos pos = {x, y, z};
                auto it = m_world.m_chunks.find(pos);

                if (it != m_world.m_chunks.end()) {
                    auto& chunk = it->second;
                    if ( !chunk->m_is_meshing and !chunk->m_is_generating and !m_chunk_meshes.contains(pos) and are_chunk_neighbours_ready(pos)) {
                        m_queue_to_mesh.push_back(pos);
                        chunk->m_is_meshing = true;
                        needs_sorting = true;
                    }
                }
            }
        }
    }

    if (needs_sorting) {
        std::sort(m_queue_to_mesh.begin(), m_queue_to_mesh.end(),
            [chunk_pos](const World::ChunkPos& a, const World::ChunkPos& b) {

                int distA = (a.x - chunk_pos.x)*(a.x - chunk_pos.x) + (a.z - chunk_pos.z)*(a.z - chunk_pos.z);
                int distB = (b.x - chunk_pos.x)*(b.x - chunk_pos.x) + (b.z - chunk_pos.z)*(b.z - chunk_pos.z);


                return distA > distB;
        });
    }

    int max_sends = 36;
    int sent_this_frame = 0;
    while (!m_queue_to_mesh_priority.empty()){
        World::ChunkPos pos_queue = m_queue_to_mesh_priority.front();
        m_queue_to_mesh_priority.pop();
        send_chunk_to_thread(pos_queue, true);
        sent_this_frame++;
    }
    while (!m_queue_to_mesh.empty() and sent_this_frame < max_sends){
        World::ChunkPos pos_queue = m_queue_to_mesh.back();
        m_queue_to_mesh.pop_back();
        send_chunk_to_thread(pos_queue, false);
        sent_this_frame++;
    }


    int unloaded_this_frame = 0;
    int max_unloads = 4;
    while (!m_chunks_to_unload.empty() and unloaded_this_frame < max_unloads){
        World::ChunkPos to_erase = m_chunks_to_unload.back();
        m_chunks_to_unload.pop_back();
        auto it = m_chunk_meshes.find(to_erase);
        if (it != m_chunk_meshes.end()){
            it->second.unload();
            m_chunk_meshes.erase(it);
            unloaded_this_frame++;
        }
        auto it_transparent = m_chunk_meshes_transparent.find(to_erase);
        if (it_transparent != m_chunk_meshes_transparent.end()){
            it_transparent->second.unload();
            m_chunk_meshes_transparent.erase(it_transparent);
            unloaded_this_frame++;
        }
    }
}

void Renderer::render_chunks(glm::vec3 player_pos, glm::mat4 viewMatrix, int windowX, int windowY)
{
    glBindVertexArray(m_emptyVAO);
    World::ChunkPos player_chunk = World::get_chunk_position(player_pos);
    int render_distance = m_renderDistance;
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)windowX / (float)windowY, 0.01f, 1000.0f);

    Frustum frustum = extract_frustum(projection, viewMatrix);

    std::vector<std::pair<World::ChunkPos, MeshSSBO>> transparent_draw_list;



    for (auto& pair : m_chunk_meshes){
        World::ChunkPos chunk_pos = pair.first;
        if (pair.first.x >= player_chunk.x - render_distance and pair.first.x <= player_chunk.x + render_distance
                and pair.first.z >= player_chunk.z - render_distance and pair.first.z <= player_chunk.z + render_distance){


            glm::vec3 chunk_min = { float(chunk_pos.x * 16), float(chunk_pos.y * 16), float(chunk_pos.z * 16) };
            glm::vec3 chunk_max = { float(chunk_pos.x * 16 + 16), float(chunk_pos.y * 16 + 16), float(chunk_pos.z * 16 + 16) };

            if (check_aabb_against_frustum(frustum, chunk_min, chunk_max)){
                if (pair.second.GetSize() > 0){
                    glm::mat4 model(1.0f);
                    model = glm::translate(model, glm::vec3(float(chunk_pos.x * 16), float(chunk_pos.y * 16), float(chunk_pos.z * 16)));
                    glm::mat4 mvp = projection * viewMatrix * model;
                    pair.second.draw(mvp, m_shader.m_shaderProgramID, m_texture, m_grassColormap);
                }
                auto it = m_chunk_meshes_transparent.find(pair.first);
                if (it != m_chunk_meshes_transparent.end() and it->second.GetSize() > 0) {
                    transparent_draw_list.emplace_back(chunk_pos, it->second);
                }
            }

        }
    }
    std::sort(transparent_draw_list.begin(), transparent_draw_list.end(),
        [player_pos](const auto& a, const auto& b) {
            // Calculate distance to chunk 'a' center
            float dx_a = (a.first.x * 16.0f + 8.0f) - player_pos.x;
            float dy_a = (a.first.y * 16.0f + 8.0f) - player_pos.y;
            float dz_a = (a.first.z * 16.0f + 8.0f) - player_pos.z;
            float distA = dx_a*dx_a + dy_a*dy_a + dz_a*dz_a;
            // Calculate distance to chunk 'b' center
            float dx_b = (b.first.x * 16.0f + 8.0f) - player_pos.x;
            float dy_b = (b.first.y * 16.0f + 8.0f) - player_pos.y;
            float dz_b = (b.first.z * 16.0f + 8.0f) - player_pos.z;
            float distB = dx_b*dx_b + dy_b*dy_b + dz_b*dz_b;
            return distA > distB; // > means furthest chunk is at the beginning of the list
    });
    for (auto& transparent_chunk : transparent_draw_list){
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(float(transparent_chunk.first.x * 16), float(transparent_chunk.first.y * 16), float(transparent_chunk.first.z * 16)));

        glm::mat4 mvp = projection * viewMatrix * model;

        transparent_chunk.second.draw(mvp, m_shader.m_shaderProgramID, m_texture, m_grassColormap);

    }
    // rlDisableVertexArray();
    // rlDisableTexture();
    // rlDisableShader();

}




bool Renderer::are_chunk_neighbours_ready(World::ChunkPos chunk_pos){
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                World::ChunkPos target_pos = {chunk_pos.x + dx, chunk_pos.y + dy, chunk_pos.z + dz};

                if (target_pos.y < 0 || target_pos.y >= World::WORLD_CHUNK_HEIGHT) {
                    continue;
                }
                auto it = m_world.m_chunks.find(target_pos);
                if (it == m_world.m_chunks.end() or it->second->m_is_generating) {
                    return false;
                }
            }
        }
    }
    return true;
}

Renderer::MeshJob Renderer::pack_mesh_job(World::ChunkPos chunk_pos){
    MeshJob mesh_job;
    mesh_job.chunk_pos = chunk_pos;

    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int index = (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9;
                World::ChunkPos target_pos = {chunk_pos.x + dx, chunk_pos.y + dy, chunk_pos.z + dz};

                auto it = m_world.m_chunks.find(target_pos);
                if (it != m_world.m_chunks.end() and !it->second->m_is_generating) {
                    mesh_job.neighbour_chunks[index] = it->second;
                    mesh_job.do_neighbour_exists[index] = true;
                } else {
                    mesh_job.neighbour_chunks[index] = nullptr;
                    mesh_job.do_neighbour_exists[index] = false;
                }
            }
        }
    }
    // Set center shortcut for convenience
    mesh_job.center_chunk = mesh_job.neighbour_chunks[13]; //13 is the center chunk
    return mesh_job;


}

void Renderer::update_mesh_chunk(const MeshJob& mesh_job, ThreadPool::SafeQueue<MeshResult>& result_queue)
{
    std::vector<std::shared_lock<std::shared_mutex>> locks;
    locks.reserve(27);
    for (int i = 0; i < 27; ++i) {
        if (mesh_job.do_neighbour_exists[i]) {
            locks.emplace_back(mesh_job.neighbour_chunks[i]->m_block_mutex);
        }
    }
    std::vector<MeshSSBO::VoxelFaceData> opaque_faces;
    std::vector<MeshSSBO::VoxelFaceData> transparent_faces;
    opaque_faces.reserve(4096);
    transparent_faces.reserve(4096);
    World::ChunkPos chunk_pos = mesh_job.chunk_pos;
    const Block* current_block = mesh_job.center_chunk->m_blocks.data();

    for (int x = 0; x < 16; ++x){
        for (int y = 0; y < 16; ++y){
            for (int z = 0; z < 16; ++z){
                if (World::BLOCK_MATERIALS::is_solid(current_block->m_material_type)){
                    perform_culling(x, y, z, current_block->m_material_type, opaque_faces, mesh_job,
                     mesh_job.center_chunk->m_blockTemperatures[x][z], mesh_job.center_chunk->m_blockHumidities[x][z]);
                } else if (World::BLOCK_MATERIALS::is_transparent(current_block->m_material_type)){
                    perform_culling(x, y, z, current_block->m_material_type, transparent_faces, mesh_job,
                     mesh_job.center_chunk->m_blockTemperatures[x][z], mesh_job.center_chunk->m_blockHumidities[x][z]);
                }
                current_block++;
            }
        }
    }
    MeshResult mesh_result;
    mesh_result.opaque_faces = std::move(opaque_faces);
    mesh_result.transparent_faces = std::move(transparent_faces);
    mesh_result.chunk_pos = chunk_pos;
    result_queue.push(std::move(mesh_result));
}


void Renderer::perform_culling(int x, int y, int z,
                        unsigned short current_block_material,
                        std::vector<MeshSSBO::VoxelFaceData>& faces, const MeshJob& mesh_job,
                        float temperature, float humidity
                        ){
    //check if block is at chunk edge
    //check X-
    unsigned short neighbour_material = get_block_material(mesh_job, x - 1, y, z);
    if (should_render_face(current_block_material, neighbour_material)){
        add_face(2, x, y, z,current_block_material, faces, mesh_job, temperature, humidity);
    }
    //X+
    neighbour_material = get_block_material(mesh_job, x + 1, y, z);
    if (should_render_face(current_block_material, neighbour_material)) {
        add_face(3, x, y, z, current_block_material, faces, mesh_job, temperature, humidity);
    }
    // Z-
    neighbour_material = get_block_material(mesh_job, x, y, z - 1);
    if (should_render_face(current_block_material, neighbour_material)) {
        add_face(1, x, y, z, current_block_material, faces, mesh_job, temperature, humidity);
    }
    // Z+
    neighbour_material = get_block_material(mesh_job, x, y, z + 1);
    if (should_render_face(current_block_material, neighbour_material)) {
        add_face(0, x, y, z, current_block_material, faces, mesh_job, temperature, humidity);
    }
    // Y-
    neighbour_material = get_block_material(mesh_job, x, y - 1, z);
    if (should_render_face(current_block_material, neighbour_material)) {
        add_face(5, x, y, z, current_block_material, faces, mesh_job, temperature, humidity);
    }
    // Y+
    neighbour_material = get_block_material(mesh_job, x, y + 1, z);
    if (should_render_face(current_block_material, neighbour_material) or
        (current_block_material == World::BLOCK_MATERIALS::WATER and neighbour_material != World::BLOCK_MATERIALS::WATER)) {
        add_face(4, x, y, z, current_block_material, faces, mesh_job, temperature, humidity);
    }
}
bool Renderer::should_render_face(unsigned short current_material, unsigned short neighbour_material){
    if (World::BLOCK_MATERIALS::is_solid(neighbour_material)) return false;

    if (World::BLOCK_MATERIALS::is_solid(current_material)) return true;

    return current_material != neighbour_material;
}

float Renderer::getTextureLayer(unsigned short material_type, int face_id){
    float textureLayer = 0;
    if (material_type == World::BLOCK_MATERIALS::DIRT){
        textureLayer = 0;
    } else if (material_type == World::BLOCK_MATERIALS::GRASS_BLOCK){
        if (face_id <= 3){
            textureLayer = 1;
        } else if (face_id == 4){
            textureLayer = 2;
        } else if (face_id == 5){
            textureLayer = 0;
        }
    } else if (material_type == World::BLOCK_MATERIALS::STONE){
        textureLayer = 3;
    } else if (material_type == World::BLOCK_MATERIALS::OAK_PLANKS){
        textureLayer = 4;
    } else if (material_type == World::BLOCK_MATERIALS::OAK_LOG){
        if (face_id <= 3){
            textureLayer = 5;
        } else{
            textureLayer = 6;
        }
    } else if (material_type == World::BLOCK_MATERIALS::SAND){
        textureLayer = 7;
    } else if (material_type == World::BLOCK_MATERIALS::WATER){
        textureLayer = 8;
    }
    return textureLayer;
}


void Renderer::add_face(int face_id, int x, int y, int z,
                        unsigned short block_material,
                        std::vector<MeshSSBO::VoxelFaceData>& faces, const MeshJob& job,
                        float temperature, float humidity
){
    bool is_top_block_of_water = block_material == World::BLOCK_MATERIALS::WATER and get_block_material(job, x, y + 1, z) != World::BLOCK_MATERIALS::WATER;

    // Get Atlas Coordinates
    float textureLayer = getTextureLayer(block_material, face_id);

    // Calculate AO for the 4 corners
    float ao_results[4];
    for (int i = 0; i < 4; ++i){
        const auto& aov = ao_neighbors[face_id][i];
        ao_results[i] = compute_ao(job, x, y, z,
                                      aov.s1.dx, aov.s1.dy, aov.s1.dz,
                                      aov.s2.dx, aov.s2.dy, aov.s2.dz,
                                      aov.corner.dx, aov.corner.dy, aov.corner.dz, block_material);
    }

    // Pack the struct
    MeshSSBO::VoxelFaceData faceData;
    faceData.x = (float)x;
    faceData.y = (float)y;
    faceData.z = (float)z;
    faceData.faceId = (float)face_id;

    faceData.textureLayer = textureLayer;
    faceData.temperature = temperature;
    faceData.isWater = is_top_block_of_water ? 1.0f : 0.0f;
    faceData.humidity = humidity;

    faceData.ao0 = ao_results[0];
    faceData.ao1 = ao_results[1];
    faceData.ao2 = ao_results[2];
    faceData.ao3 = ao_results[3];

    faces.push_back(faceData);
}

unsigned short Renderer::get_block_material(const MeshJob& job, int x, int y, int z) {
    int dx = (x < 0) ? -1 : (x > 15 ? 1 : 0);
    int dy = (y < 0) ? -1 : (y > 15 ? 1 : 0);
    int dz = (z < 0) ? -1 : (z > 15 ? 1 : 0);

    int index = (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9;

    if (!job.do_neighbour_exists[index]) return false;

    int lx = ((x % 16) + 16) % 16;
    int ly = ((y % 16) + 16) % 16;
    int lz = ((z % 16) + 16) % 16;
    return job.neighbour_chunks[index]->m_blocks[lz + ly * 16 + lx * 256].m_material_type;
}



unsigned char Renderer::compute_ao(const MeshJob& job, int x, int y, int z,
                                   int dx1, int dy1, int dz1,   // side 1
                                   int dx2, int dy2, int dz2,   // side 2
                                   int dcx, int dcy, int dcz,
                                   unsigned int block_material)   // corner diagonal
{
    bool side1  = World::BLOCK_MATERIALS::is_solid(get_block_material(job, x + dx1, y + dy1, z + dz1));
    bool side2  = World::BLOCK_MATERIALS::is_solid(get_block_material(job, x + dx2, y + dy2, z + dz2));
    bool corner = World::BLOCK_MATERIALS::is_solid(get_block_material(job, x + dcx, y + dcy, z + dcz));

    int ao;
    if (side1 && side2)
        ao = 0;
    else
        ao = 3 - (int(side1) + int(side2) + int(corner));

    constexpr unsigned char ao_table[4] = { 140, 175, 210, 255 };
    return ao_table[ao];
}


void Renderer::upload_mesh_to_gpu(const MeshResult& mesh_result){
    World::ChunkPos chunk_pos = mesh_result.chunk_pos;
    if (!m_world.m_chunks.contains(chunk_pos)) return;

    if (m_chunk_meshes.contains(chunk_pos)){
            m_chunk_meshes[chunk_pos].unload();
            m_chunk_meshes.erase(chunk_pos);
        }
    MeshSSBO opaqueMesh;
    if (!mesh_result.opaque_faces.empty()){
        opaqueMesh.upload(mesh_result.opaque_faces);
    }
    m_chunk_meshes[chunk_pos] = opaqueMesh;

    if (m_chunk_meshes_transparent.contains(chunk_pos)){
        m_chunk_meshes_transparent[chunk_pos].unload();
        m_chunk_meshes_transparent.erase(chunk_pos);
    }

    MeshSSBO transparentMesh;
    if (!mesh_result.transparent_faces.empty()){
        transparentMesh.upload(mesh_result.transparent_faces);
    }
    m_chunk_meshes_transparent[chunk_pos] = transparentMesh;



    auto it = m_world.m_chunks.find(chunk_pos);
    if (it != m_world.m_chunks.end()) it->second->m_is_meshing = false;
}



bool Renderer::send_chunk_to_thread(World::ChunkPos chunk_pos, bool is_priority){
    MeshJob mesh_job = pack_mesh_job(chunk_pos);
    if (!mesh_job.center_chunk){
        auto it = m_world.m_chunks.find(chunk_pos);
        if (it != m_world.m_chunks.end()) {
            it->second->m_is_meshing = false;
        }
        return false;
    }


    if (!is_priority){
        m_threadPool.enqueue([this, mesh_job = std::move(mesh_job)](){
           this->update_mesh_chunk(mesh_job, m_result_queue);
        });
    } else{
        m_threadPool.enqueue([this, mesh_job = std::move(mesh_job)](){
           this->update_mesh_chunk(mesh_job, m_result_queue_priority);
        });
    }
    return true;
}



void Renderer::update_block_meshes(World::ChunkPos chunk_pos, int local_x, int local_y, int local_z) {


    auto queue_chunk = [this](World::ChunkPos pos) {
        if (m_world.m_chunks.count(pos) > 0) {
            auto& chunk = m_world.m_chunks[pos];
            if (chunk && !chunk->m_is_meshing) {
                m_queue_to_mesh_priority.emplace(pos);
                chunk->m_is_meshing = true;
            }
        }
    };


    queue_chunk(chunk_pos);


    if (local_x == 0)  queue_chunk({chunk_pos.x - 1, chunk_pos.y, chunk_pos.z});
    if (local_x == 15) queue_chunk({chunk_pos.x + 1, chunk_pos.y, chunk_pos.z});

    if (local_y == 0)  queue_chunk({chunk_pos.x, chunk_pos.y - 1, chunk_pos.z});
    if (local_y == 15) queue_chunk({chunk_pos.x, chunk_pos.y + 1, chunk_pos.z});

    if (local_z == 0)  queue_chunk({chunk_pos.x, chunk_pos.y, chunk_pos.z - 1});
    if (local_z == 15) queue_chunk({chunk_pos.x, chunk_pos.y, chunk_pos.z + 1});
}

Renderer::Frustum Renderer::extract_frustum(const glm::mat4& proj, const glm::mat4& view) {
    Frustum f;

    glm::mat4 vp = proj * view;

    // f.planes[0] = {vp.m3 + vp.m0, vp.m7 + vp.m4, vp.m11 + vp.m8,  vp.m15 + vp.m12}; // Left
    // f.planes[1] = {vp.m3 - vp.m0, vp.m7 - vp.m4, vp.m11 - vp.m8,  vp.m15 - vp.m12}; // Right
    // f.planes[2] = {vp.m3 + vp.m1, vp.m7 + vp.m5, vp.m11 + vp.m9,  vp.m15 + vp.m13}; // Bottom
    // f.planes[3] = {vp.m3 - vp.m1, vp.m7 - vp.m5, vp.m11 - vp.m9,  vp.m15 - vp.m13}; // Top
    // f.planes[4] = {vp.m3 + vp.m2, vp.m7 + vp.m6, vp.m11 + vp.m10, vp.m15 + vp.m14}; // Near
    // f.planes[5] = {vp.m3 - vp.m2, vp.m7 - vp.m6, vp.m11 - vp.m10, vp.m15 - vp.m14}; // Far

    // Left
    f.planes[0] = { vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0] };
    // Right
    f.planes[1] = { vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0] };
    // Bottom
    f.planes[2] = { vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1] };
    // Top
    f.planes[3] = { vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1] };
    // Near
    f.planes[4] = { vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2] };
    // Far
    f.planes[5] = { vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2] };

    // Normalize each plane for accurate distance calculations
    for (int i = 0; i < 6; i++) {
        float length = sqrt(f.planes[i].x * f.planes[i].x +
                            f.planes[i].y * f.planes[i].y +
                            f.planes[i].z * f.planes[i].z);
        f.planes[i].x /= length;
        f.planes[i].y /= length;
        f.planes[i].z /= length;
        f.planes[i].d /= length;
    }

    return f;
}

bool Renderer::check_aabb_against_frustum(const Frustum& frustum, glm::vec3 min, glm::vec3 max) {
    for (int i = 0; i < 6; i++) {
        glm::vec3 p = min;
        if (frustum.planes[i].x >= 0) p.x = max.x;
        if (frustum.planes[i].y >= 0) p.y = max.y;
        if (frustum.planes[i].z >= 0) p.z = max.z;

        if (frustum.planes[i].x * p.x +
            frustum.planes[i].y * p.y +
            frustum.planes[i].z * p.z +
            frustum.planes[i].d < -0.1f) {
            return false;
        }
    }
    return true;
}