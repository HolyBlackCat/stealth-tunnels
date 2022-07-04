#include "main.h"

#include "game/map.h"

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map;

        void Init() override
        {
            map = Map::LoadFromFile(Program::ExeDir() + "assets/maps/test.json");
        }

        void Tick(std::string &next_state) override
        {
            (void)next_state;


        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            map.Render(mouse.pos());

            r.Finish();
        }
    };
}
