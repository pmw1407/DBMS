SELECT type, COUNT(*) AS C
FROM Pokemon
GROUP BY type
ORDER BY C, type;