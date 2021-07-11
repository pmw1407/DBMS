SELECT name
FROM Pokemon
WHERE type IN(
  SELECT type
  FROM(
    SELECT MAX(result.num) AS second
    FROM (
      SELECT type, COUNT(id) AS num
      FROM Pokemon
      GROUP BY type) AS result
      WHERE result.num < (SELECT MAX(result.num)
                          FROM (SELECT type, COUNT(id) AS num
                                FROM Pokemon
                                GROUP BY type) AS result)) AS Result, (SELECT type, COUNT(id) AS n
                                                                       FROM Pokemon
                                                                       GROUP BY type) AS temp
  WHERE temp.n >= Result.second)
ORDER BY name;