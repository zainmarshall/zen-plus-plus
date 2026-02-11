---
title: "Day0"
author: "Zain Marshall"
date: "2026-02-09"
output: pdf_document
---

# Problem 1
```{r}
mileage <- c(65311, 65624, 65908, 66219, 66499, 66821, 67145, 67447)
diffs <- mileage[2:8] - mileage[1:7]
max(diffs) # 324
mean(diffs) # 305.1429
min(diffs) # 280
```

# Problem 2
```{r}
commutes <- c(17, 16, 20, 24, 22, 15, 21, 15, 17, 22)
```