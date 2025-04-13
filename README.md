# The Water Problem

**Goal**  
Given an n x n grid, we can initially place water in any cells we choose. Then, at each timestep, every cell that has two or more water-filled neighbors (direct neighbors only: top, bottom, left, right) becomes water-filled itself. The process continues until no more cells change. Our challenge is to find the initial placement(s) of water that maximizes the timestep at which the system stabilizes.

For each n, we define f(n) as the largest “final” timestep number in the grid after simulation, achieved by some initial water arrangement.

## Known Results

1. You can find our Definitions [here](DEFINITIONS.md).
2. You can find our Theorems [here](THEOREMS.md).
3. Table of Known f(n) Values
    | **n**  | 1  | 2  | 3  | 4   | 5   | 6   | 7    | 8    | 9    |
    |:------:|:--:|:--:|:--:|:---:|:---:|:---:|:----:|:----:|:----:|
    |**f(n)**| 1  | 2  | 5  | 10  | 16  | 23  | ≥ 31 | ≥ 41 | ≥ 52 |

---

## Project Resources

- **Join Our WhatsApp Group**  
  If you’d like to contribute or discuss the project, please join the associated WhatsApp group (found in our community) for real-time collaboration.

- **Interactive Simulation**  
  There is an online simulation demonstrating the water spread at:  
  [https://recmaths.ch/problems/water/](https://recmaths.ch/problems/water/)

- **C-Implementation**  
  In the [`code-implementations`](code-implementations) directory, you’ll find the programming-based approaches we used to establish values of f(n).

- **Research Paper**  
  A paper that studies this problem in depth can be found here:  
  [https://arxiv.org/pdf/1310.4457](https://arxiv.org/pdf/1310.4457)

---

## Contributing

1. If you want to add Defintions and/or Theorems of your own, see the existing results first.
2. Edit the relevant files using the GitHub Interface (or any other Git interface) and create a pull request.
3. If you're unsure about anything or would like additional help, text in the relevant WhatsApp group!
