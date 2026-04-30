#version 430 core

// 1. Memory-aligned blueprint matching our C++ struct exactly
struct VoxelFaceData {
    vec4 positionAndFace; // x, y, z, faceId
    vec4 atlasAndPadding; // atlasX, atlasY, pad, pad
    vec4 ao;              // ao0, ao1, ao2, ao3
};

// 2. The SSBO
layout(std430, binding = 0) buffer FaceBuffer {
    VoxelFaceData faces[];
};

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;

// --- YOUR EXACT C++ ARRAYS MAPPED TO GLSL ---

// Renderer.h: m_face_vertices flattened into 24 points
const vec3 FACE_VERTICES[24] = vec3[](
// 0: Front (Z+)
vec3(0,0,1), vec3(1,0,1), vec3(1,1,1), vec3(0,1,1),
// 1: Back (Z-)
vec3(1,0,0), vec3(0,0,0), vec3(0,1,0), vec3(1,1,0),
// 2: Left (X-)
vec3(0,0,0), vec3(0,0,1), vec3(0,1,1), vec3(0,1,0),
// 3: Right (X+)
vec3(1,0,1), vec3(1,0,0), vec3(1,1,0), vec3(1,1,1),
// 4: Top (Y+)
vec3(0,1,1), vec3(1,1,1), vec3(1,1,0), vec3(0,1,0),
// 5: Bottom (Y-)
vec3(0,0,0), vec3(1,0,0), vec3(1,0,1), vec3(0,0,1)
);

// Renderer.h: m_face_UVs
const vec2 FACE_UVS[4] = vec2[](
vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 0.0)
);

// Renderer.h: m_shades (converted to normalized 0.0 - 1.0 floats)
const float FACE_SHADES[6] = float[](
200.0/255.0, 160.0/255.0, 200.0/255.0, 160.0/255.0, 255.0/255.0, 130.0/255.0
);

void main() {
    int blueprintIndex = gl_VertexID / 6;
    int vertexInFace   = gl_VertexID % 6;

    VoxelFaceData face = faces[blueprintIndex];
    int faceId = int(face.positionAndFace.w);

    // --- YOUR EXACT C++ AO INDEX FLIPPING LOGIC ---
    int uniqueCornerId;
    if (face.ao.x + face.ao.z > face.ao.y + face.ao.w) {
        int pattern[6] = int[](0, 1, 2, 2, 3, 0); // Triangle setup 1
        uniqueCornerId = pattern[vertexInFace];
    } else {
        int pattern[6] = int[](1, 2, 3, 3, 0, 1); // Triangle setup 2
        uniqueCornerId = pattern[vertexInFace];
    }

    // 1. Calculate Vertex Position
    int lookupIndex = (faceId * 4) + uniqueCornerId;
    vec3 localPos = FACE_VERTICES[lookupIndex];

    // Check for water logic (you had a y = 0.875f check in add_face).
    // You can pass a flag in pad1 to handle this!
    if (face.atlasAndPadding.z > 0.0 && localPos.y == 1.0) localPos.y = 0.875;


    vec3 finalPos = face.positionAndFace.xyz + localPos;
    gl_Position = mvp * vec4(finalPos, 1.0);

    // 2. Calculate UVs
    fragTexCoord = (FACE_UVS[uniqueCornerId] + face.atlasAndPadding.xy);

    // 3. Calculate Final Vertex Color (Base Shade * AO)
    // Extract the specific AO corner we are rendering right now
    float cornerAO;
    if      (uniqueCornerId == 0) cornerAO = face.ao.x;
    else if (uniqueCornerId == 1) cornerAO = face.ao.y;
    else if (uniqueCornerId == 2) cornerAO = face.ao.z;
    else                          cornerAO = face.ao.w;

    float combinedShade = FACE_SHADES[faceId] * (cornerAO / 255.0);
    fragColor = vec4(combinedShade, combinedShade, combinedShade, 1.0);
}

