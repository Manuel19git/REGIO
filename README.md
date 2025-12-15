# REGIO Engine
This is an engine done in c++ using directx11 for learning purposes.

## Milestones
A simple scene with geometry uploaded from a single Buffer. When assets are going to be rendered in the same way by the same shaders, it would remove the need to make more than one draw call
<p align="center">
  <img src="https://drive.google.com/uc?export=view&id=1BNav2vd4zeWy-gZFXU7S0wSZtF5bDZZ3"/>
</p>

Here I wanted to implement shadows using shadow maps. Because I had the sun as the source of light, I generate the shadow map using a camera with orthographic projection. 
<p align="center">
  <img src="https://drive.google.com/uc?export=view&id=15PIGEVPegkkN5j_TM22rPNzAmCpUTYYV"/>
</p>

Having a shadow map with larger size would make the shadows look smoother, but I wanted to keep developing the engine on a not so powerfull laptop and I implemented PCF to smooth a bit the shadow edges
<p align="center">
  <img src="https://drive.google.com/uc?export=view&id=1Qht5ZtaQpSmi4UXZ3yS2nLWC6gLokqbw"/>
</p>


## Future plans
Currently I am in the middle of abstracting elements of the core engine to be able to add new rendering APIs in the future. I am in the middle of developing something ECS-ish with a clear speration of data and behaviour, using Render Item object to store handlers to (mesh,material,transform,etc). But this will not be a full implementation of an Entity Component System yet. 
