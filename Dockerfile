FROM --platform=linux/amd64 ubuntu:20.04

# Evitar prompts durante la instalación
ARG DEBIAN_FRONTEND=noninteractive

# Instalar herramientas necesarias
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Instalar dependencias de Python
RUN pip3 install streamlit

# Configurar directorio de trabajo
WORKDIR /app

# Copiar archivos del proyecto
COPY . /app/

# Crear directorio para resultados
RUN mkdir -p /app/frontend/resultado

# Puerto para Streamlit
EXPOSE 8501

# Comando para ejecutar la aplicación
CMD ["python3", "-m", "streamlit", "run", "frontend/frontend_app.py", "--server.address=0.0.0.0"]
