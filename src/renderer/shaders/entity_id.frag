#version 410 core

layout(location = 0) out uint entity_id_output;

uniform uint entity_id;

void main()
{
    entity_id_output = entity_id;
}
