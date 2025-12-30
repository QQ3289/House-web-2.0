# Copilot Instructions

- Project goal: Qt6-based Linux web application providing second-hand housing info services; spec is in [提示词.txt](提示词.txt).
- Platform: Ubuntu (Aliyun ECS), Qt6 with WebEngine for the web UI; back end also in Qt; MySQL as persistence.
- Core data source: MySQL database `house_db`, existing table `houseinfo` with columns `(ID int auto inc PK, houseTitle varchar(100), price double, area double, communityNa varchar(255), floor varchar(50) nullable, houseType varchar(50), unitPrice double, houseUrl varchar(1024))`; do not alter this table, add new tables as needed.
- Features to support: guest browsing; user accounts with register/login, email verification on sign-up, password change with email notice; developer/admin account for user management and viewing stats by user persona.
- Housing flows: browse listings from MySQL; filtering by total price, unit price, region, area, house type; detail view links to source URL and shows Baidu Map location; ability to generate statistical views/visualizations of housing data.
- AI assistant: integrate DeepSeek paid API; must accept user needs, query MySQL, and return purchase recommendations; keep API credentials in config file (not hardcoded).
- Configuration: single JSON file should hold MySQL connection, DeepSeek API, and Baidu Map API settings; load this config in both UI and data layers.
- Web UI: built with Qt WebEngine; emphasize attractive UI per spec; ensure detail view includes map embed and stats pages are reachable.
- Deployment: generate only an Ubuntu (Aliyun ECS) deployment guide; no other platforms.
- Conventions: stick to ASCII unless existing file uses non-ASCII; comments only where code isn’t self-explanatory.
- Security: never log or commit secrets; read credentials from the JSON config and support environment overrides if added later.
- Testing/dev workflows: when introducing build/test scripts, target Qt6 on Ubuntu with MySQL client libraries present.
- External integrations: Baidu Maps for geolocation in detail views; DeepSeek for AI recommendations; MySQL for all data. Avoid inventing other providers unless the spec expands.
- UI behaviors: detail click should navigate to full listing page with all fields, map, and source link; filters should combine region/price/area/houseType without breaking pagination; favorites/preferences must persist to DB.
- Analytics: developer/admin view should surface user-account management and persona-based statistics; housing statistics views should pull from MySQL (no mock data).
- Keep README/deployment docs aligned with the Ubuntu/Qt6/MySQL/WebEngine stack; avoid generic cross-platform instructions.
