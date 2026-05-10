#pragma once
#include <glad/glad.h>

#include "BlockRegistry.h"
#include "MeshSSBO.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "World.h"
#include "ThreadPool.h"
class Renderer {
public:
    Renderer(World &worldInstance, ThreadPool& threadPoolInstance, int renderDistance, BlockRegistry &blockRegistryInstance);
    void init();
    // void render(glm::mat4 viewMatrix);
private:
    GLuint m_emptyVAO;
    Shader m_shader;
    GLuint m_texture = 0;
    GLuint m_grassColormap = 0;
    void loadTextures();
    void setupBuffers();
public:
    void update_meshes(glm::vec3 player_pos);
    void render_chunks(glm::vec3 player_pos, glm::mat4 viewMatrix, int windowX, int windowY);
    // Renderer(){}
    // for World.cpp so that chunks are erased and unloaded at the same time
    void add_chunk_to_unload(World::ChunkPos pos){
        m_chunks_to_unload.push_back(pos);
    }
    // remeshes needed chunks after world interaction
    void update_block_meshes(World::ChunkPos chunk_pos, int local_x, int local_y, int local_z);
    std::unordered_map<World::ChunkPos, MeshSSBO, World::ChunkPosHash> m_chunk_meshes;
    std::unordered_map<World::ChunkPos, MeshSSBO, World::ChunkPosHash> m_chunk_meshes_transparent;

private:
    int m_renderDistance;
    World& m_world;
    ThreadPool& m_threadPool;
    BlockRegistry& m_blockRegistry;
    std::vector<MeshSSBO::VoxelFaceData> m_faces;
    struct MeshResult{
        std::vector<MeshSSBO::VoxelFaceData> opaque_faces;
        std::vector<MeshSSBO::VoxelFaceData> transparent_faces;
        World::ChunkPos chunk_pos;
    };
    struct MeshJob{
        World::ChunkPos chunk_pos;
        std::shared_ptr<Chunk> center_chunk;
        std::shared_ptr<Chunk> neighbour_chunks[27]; //13 neighbour is the center_chunk
        bool do_neighbour_exists[27];
    };

    bool are_chunk_neighbours_ready(World::ChunkPos chunk_pos);
    MeshJob pack_mesh_job(World::ChunkPos chunk_pos);
    void update_mesh_chunk(const MeshJob& mesh_job, ThreadPool::SafeQueue<MeshResult>& result_queue);
    void perform_culling(int x, int y, int z, unsigned short current_block_material,
                         std::vector<MeshSSBO::VoxelFaceData>& faces, const MeshJob& mesh_job, float temperature, float humidity);

    void add_face(int face_id, int x, int y, int z, unsigned short block_material,
                  std::vector<MeshSSBO::VoxelFaceData>& faces, const MeshJob& job, float temperature, float humidity);
    bool should_render_face(unsigned short current_material, unsigned short neighbour_material);
    bool send_chunk_to_thread(World::ChunkPos chunk_pos, bool is_priority);
    ThreadPool::SafeQueue<MeshResult> m_result_queue;
    ThreadPool::SafeQueue<MeshResult> m_result_queue_priority;
    std::vector<World::ChunkPos> m_queue_to_mesh;
    std::queue<World::ChunkPos> m_queue_to_mesh_priority;
    World::ChunkPos m_last_player_chunk;
    std::vector<World::ChunkPos> m_chunks_to_unload;
    float getTextureLayer(unsigned short material_type, int face_id);
    void upload_mesh_to_gpu(const MeshResult& mesh_result);
    unsigned char compute_ao(const MeshJob& job, int x, int y, int z, int dx1, int dy1, int dz1, int dx2, int dy2,
                             int dz2,
                             int dcx, int dcy, int dcz, unsigned int block_material);
    unsigned short get_block_material(const MeshJob& job, int x, int y, int z);

    struct AONeighbor { int dx, dy, dz; };
    struct AOVertex { AONeighbor s1, s2, corner; };

    const AOVertex ao_neighbors[6][4] = {
        // Face 0: Z+ (front, normal +Z)
        { { {-1,0,1},{0,-1,1},{-1,-1,1} },
            { {1,0,1},{0,-1,1},{1,-1,1} },
          { {1,0,1},{0,1,1},{1,1,1} },
            { {-1,0,1},{0,1,1},{-1,1,1} } },
        // Face 1: Z- (back, normal -Z)
        { { {1,0,-1},{0,-1,-1},{1,-1,-1} },
            { {-1,0,-1},{0,-1,-1},{-1,-1,-1} },
          { {-1,0,-1},{0,1,-1},{-1,1,-1} },
            { {1,0,-1},{0,1,-1},{1,1,-1} } },
        // Face 2: X- (left, normal -X)
        {
                { {-1,0,-1}, {-1,-1,0}, {-1,-1,-1} },
                { {-1,0,1}, {-1,-1,0}, {-1,-1,1} },
                { {-1,0,1}, {-1,1,0}, {-1,1,1} },
                { {-1,0,-1}, {-1,1,0}, {-1,1,-1} }
        },

        // Face 3: X+ (right, normal +X)
        {
                { {1,0,1}, {1,-1,0}, {1,-1,1} },
                { {1,0,-1}, {1,-1,0}, {1,-1,-1} },
                { {1,0,-1}, {1,1,0}, {1,1,-1} },
                { {1,0,1}, {1,1,0}, {1,1,1} }
        },
        // Face 4: Y+ (top, normal +Y)
    {
            { {-1,1,0},{0,1,1},{-1,1,1} },  // V0 (0,1,1): Left & Front
            { {1,1,0},{0,1,1},{1,1,1} },   // V1 (1,1,1): Right & Front
            { {1,1,0},{0,1,-1},{1,1,-1} }, // V2 (1,1,0): Right & Back
            { {-1,1,0},{0,1,-1},{-1,1,-1} } // V3 (0,1,0): Left & Back
    },

    // Face 5: Y- (bottom, normal -Y)
    {
            { {-1,-1,0},{0,-1,-1},{-1,-1,-1} },// V1 (0,0,0): Left & Back
            { {1,-1,0},{0,-1,-1},{1,-1,-1} }, // V0 (1,0,0): Right & Back
            { {1,-1,0},{0,-1,1},{1,-1,1} },    // V3 (1,0,1): Right & Front
            { {-1,-1,0},{0,-1,1},{-1,-1,1} }  // V2 (0,0,1): Left & Front
    },
    };
    const glm::vec3 m_face_vertices[6][4]{
        { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} }, // Front (Z+)
        { {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} }, // Back (Z-)
        { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0} }, // Left (X-)
        { {1,0,1}, {1,0,0}, {1,1,0}, {1,1,1} }, // Right (X+)
        { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} }, // Top (Y+)
        { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} }  // Bottom (Y-)
    };
    const unsigned short m_face_indices[6]
    {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    const float m_face_UVs[4][2]
    {
        {1.0f, 1.0f}, // Top-Right
        {0.0f, 1.0f}, // Top-Left
            {0.0f, 0.0f}, // Bottom-Left  u  v
        {1.0f, 0.0f} // Bottom-Right
    };
    const unsigned char m_shades[6]{
        200,
        160,
        200,
        160,
        255,
        130
    };
    struct FrustumPlane {
        float x, y, z, d;
    };
    struct Frustum {
        FrustumPlane planes[6];
    };
    Frustum extract_frustum(const glm::mat4& proj, const glm::mat4& view);
    bool check_aabb_against_frustum(const Frustum& frustum, glm::vec3 min, glm::vec3 max);
};
