# Basic HTTP Server made in C
This is a learning project. This server is missing A LOT of things, it's only made to start and retrieve HTML files and a favicon.
## Configuration
I created a custom configuration file, it's pretty simple to understand, go check `config.example` it's the configuration for the default website. Rename it to `config` to make it work ! There is still an error when using the server without config file (to reproduce, request a direct file not a route, i.e. `/index.html`).
