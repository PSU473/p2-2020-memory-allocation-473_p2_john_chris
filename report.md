Project Members: John Rost and Chris Muller ||
Report Author: John Rost || 
  For our second project of the semester, we were tasked with implementing the slab and buddy system malloc functionality in C. In order to accomplish this task, 
  a great deal of research was performed by both myself and Chris Muller. I focused mainly on the slab functionality, while Chris researched the buddy system and 
  possible ways to implement it, and we frequently got in touch with eachother to discuss how we might tackle both problems. I handled most of the coding while
  Chris helped me with conceptually implementing the required functions. The biggest challenge I faced was undoubtedly getting the free_buddy function to work, as
  I was frequently encountering Seg Faults while trouble shooting my code. I noticed some strange behavior with my if statements, where if my conditions were ordered
  a certain way a Seg Fault would occur, however if they were ordered another way my code would work flawlessly; I still do not quite understand that issue. It was 
  also fairly difficult figuring out how to designate two memory chunks as buddies while trying to free space, however I was eventually able to achieve this through a 
  few simple if statements. The structure of our code is pretty straight forward: we defined several structs to help build our linked lists of slabs and buddys, initialized
  our slabs and free_list in the setup function, and implemented our buddy system malloc and free functions before finishing our slab functionality (as it was necessary to 
  use the code for our buddy functions within our slab functionality). Despite a lot of time spent trouble shooting small errors, the lab went pretty smoothly and we are 
  both proud of the work we accomplished.
