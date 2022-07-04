#include "lighting.h"

#include "game/main.h"

namespace Lighting
{
    struct Shader
    {
        SIMPLE_STRUCT( Uniforms
            DECL(Graphics::Uniform<fvec2> ATTR Graphics::Vert) camera_pos // In map pixels.
            DECL(Graphics::Uniform<fvec2> ATTR Graphics::Vert) camera_scale
            DECL(Graphics::Uniform<fvec2> ATTR Graphics::Vert) center // In map pixels.
        )

        Uniforms uni; // Must be before the `shader` to be constructed first.

        Graphics::Shader shader;
    };

    Shader &GetShader()
    {
        static Shader ret = []{
            Shader ret{{}, Graphics::Shader("Lighting", shader_config, {}, Meta::tag<Attribs>{}, ret.uni, R"(
void main()
{
    vec2 pos = a_pos.xy + (a_pos.xy - u_center) * (a_pos.z * 1000);
    gl_Position = vec4((pos - u_camera_pos) * u_camera_scale, 0, 1);
}
)", R"(
void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
}
)")};
            ret.uni.camera_scale = fvec2(2, -2) / screen_size;
            return ret;
        }();

        return ret;
    }

    EdgeLoops::EdgeLoops(std::function<void(VertexInputCallback callback)> func)
    {
        std::optional<ivec2> prev_pos;
        std::vector<Attribs> vertices;

        func([&](ivec2 pos, bool last)
        {
            if (prev_pos)
            {
                Attribs a{prev_pos->to_vec3(0)};
                Attribs b{      pos.to_vec3(0)};
                Attribs c{      pos.to_vec3(1)};
                Attribs d{prev_pos->to_vec3(2)};

                vertices.push_back(a);
                vertices.push_back(b);
                vertices.push_back(c);

                vertices.push_back(a);
                vertices.push_back(c);
                vertices.push_back(d);
            }

            if (last)
                prev_pos.reset();
            else
                prev_pos = pos;
        });

        vertex_buffer = Graphics::VertexBuffer<Attribs>(vertices.size(), vertices.data());
    }

    void EdgeLoops::Render(ivec2 camera_pos, ivec2 light_pos) const
    {
        Shader &shader = GetShader();
        shader.shader.Bind();
        shader.uni.camera_pos = camera_pos;
        // `camera_scale` is set during init.
        shader.uni.center = light_pos;

        vertex_buffer.Draw(Graphics::triangles);
    }
}
