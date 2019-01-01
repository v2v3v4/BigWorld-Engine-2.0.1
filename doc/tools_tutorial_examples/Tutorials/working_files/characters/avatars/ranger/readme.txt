1. A walk_backwards animation is derived from a walk_forward animation played in reverse. 
2. The Turn animations have a small amount of translation in their root node. This is required in 1.9 but has been fixed in 2.0.
3. The ranger is an unusual asset in that it faces forward, yet its animations run in the other direction. This is an unnecessary complication. It is perfectly fine to have your model facing either direction, and animate in either direction. The "opposite facing" tag in both the animation and visual exporter can reverse either the model or the animation data. 
