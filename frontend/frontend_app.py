import streamlit as st
import os
import tempfile
from executer import compile_go_code, compile_assembly_to_executable, run_executable, load_test_examples

# Configuración de la página
st.set_page_config(
    page_title="Compilador Go a x86_64",
    page_icon="🖥️",
    layout="wide",
    initial_sidebar_state="expanded"
)

# CSS personalizado
st.markdown("""
<style>
    .main-title {
        text-align: center;
        font-size: 3em;
        font-weight: bold;
        margin-bottom: 30px;
        color: #0066cc;
        text-shadow: 2px 2px 4px #cccccc;
    }
    .section-title {
        font-size: 1.8em;
        font-weight: bold;
        margin-top: 20px;
        color: #333333;
        border-bottom: 2px solid #0066cc;
        padding-bottom: 10px;
    }
    .success {
        background-color: #d4edda;
        border: 1px solid #c3e6cb;
        color: #155724;
        padding: 10px;
        border-radius: 5px;
        margin: 10px 0;
    }
    .error {
        background-color: #f8d7da;
        border: 1px solid #f5c6cb;
        color: #721c24;
        padding: 10px;
        border-radius: 5px;
        margin: 10px 0;
    }
    .code-container {
        border: 1px solid #e1e4e8;
        border-radius: 6px;
        padding: 10px;
        background-color: #f6f8fa;
    }
    .footer {
        text-align: center;
        margin-top: 50px;
        color: #6c757d;
        font-size: 0.9em;
    }
</style>
""", unsafe_allow_html=True)

# Título y descripción
st.markdown('<div class="main-title">Compilador de Go a Ensamblador x86_64</div>', unsafe_allow_html=True)
st.markdown("""
Este compilador traduce un subconjunto del lenguaje Go a código ensamblador x86_64, 
permitiendo la ejecución de programas Go simples en arquitecturas compatibles.
""")

# Sidebar
st.sidebar.title("Navegación")
options = ["Compilador", "Ejemplos", "Preguntas Frecuentes"]
selection = st.sidebar.radio("Ir a:", options)

# Cargar ejemplos
examples = load_test_examples()
example_names = list(examples.keys())

if selection == "Compilador":
    st.markdown('<div class="section-title">Compilador</div>', unsafe_allow_html=True)
    
    # Pestañas para diferentes métodos de entrada
    input_method = st.tabs(["Escribir código", "Subir archivo", "Seleccionar ejemplo"])
    
    with input_method[0]:
        go_code = st.text_area("Escribe tu código Go aquí:", height=300, 
                              value='package main\n\nimport "fmt"\n\nfunc main() {\n    fmt.Println("Hello, World!")\n}')
    
    with input_method[1]:
        uploaded_file = st.file_uploader("Sube un archivo .go o .txt", type=['go', 'txt'])
        if uploaded_file is not None:
            go_code = uploaded_file.getvalue().decode("utf-8")
            st.markdown('<div class="code-container">', unsafe_allow_html=True)
            st.code(go_code, language='go')
            st.markdown('</div>', unsafe_allow_html=True)
    
    with input_method[2]:
        if example_names:
            selected_example = st.selectbox("Selecciona un ejemplo:", example_names)
            go_code = examples[selected_example]
            st.markdown('<div class="code-container">', unsafe_allow_html=True)
            st.code(go_code, language='go')
            st.markdown('</div>', unsafe_allow_html=True)
        else:
            st.warning("No se encontraron ejemplos disponibles.")
    
    # Botón para compilar
    if st.button("Compilar y Ejecutar"):
        with st.spinner("Compilando y ejecutando..."):
            # Paso 1: Compilar Go a ensamblador
            success, message, asm_path = compile_go_code(go_code)
        
        if success:
            st.markdown(f'<div class="success">{message}</div>', unsafe_allow_html=True)
            
            # Mostrar contenido del archivo ensamblador generado
            try:
                with open(os.path.join("resultado", "result.s"), 'r') as f:
                    assembly_code = f.read()
                st.markdown("### Código ensamblador generado:")
                st.markdown('<div class="code-container">', unsafe_allow_html=True)
                st.code(assembly_code, language='asm')
                st.markdown('</div>', unsafe_allow_html=True)
                
                # Paso 2: Compilar ensamblador a ejecutable
                with st.spinner("Compilando a ejecutable..."):
                    success_exe, message_exe, exe_path = compile_assembly_to_executable(asm_path)
                
                if success_exe:
                    st.markdown(f'<div class="success">{message_exe}</div>', unsafe_allow_html=True)
                    
                    # Paso 3: Ejecutar el programa
                    with st.spinner("Ejecutando programa..."):
                        success_run, output, error = run_executable(exe_path)
                    
                    if success_run:
                        st.markdown("### Resultado de la ejecución:")
                        st.markdown('<div class="code-container" style="background-color: #f0f0f0;">', unsafe_allow_html=True)
                        st.text(output)
                        st.markdown('</div>', unsafe_allow_html=True)
                    else:
                        st.markdown(f'<div class="error">Error al ejecutar: {error}</div>', unsafe_allow_html=True)
                else:
                    st.markdown(f'<div class="error">{message_exe}</div>', unsafe_allow_html=True)
            except Exception as e:
                st.error(f"Error al procesar el archivo: {str(e)}")
        else:
            st.markdown(f'<div class="error">{message}</div>', unsafe_allow_html=True)

elif selection == "Ejemplos":
    st.markdown('<div class="section-title">Ejemplos de código Go</div>', unsafe_allow_html=True)
    
    if example_names:
        selected_example = st.selectbox("Selecciona un ejemplo:", example_names)
        st.markdown(f"### {selected_example}")
        st.markdown('<div class="code-container">', unsafe_allow_html=True)
        st.code(examples[selected_example], language='go')
        st.markdown('</div>', unsafe_allow_html=True)
        
        # Opción para compilar el ejemplo seleccionado
        if st.button("Compilar y Ejecutar este ejemplo"):
            with st.spinner("Compilando..."):
                success, message, asm_path = compile_go_code(examples[selected_example])
            
            if success:
                st.markdown(f'<div class="success">{message}</div>', unsafe_allow_html=True)
                
                # Mostrar contenido del archivo ensamblador generado
                try:
                    with open(os.path.join("resultado", "result.s"), 'r') as f:
                        assembly_code = f.read()
                    st.markdown("### Código ensamblador generado:")
                    st.markdown('<div class="code-container">', unsafe_allow_html=True)
                    st.code(assembly_code, language='asm')
                    st.markdown('</div>', unsafe_allow_html=True)
                    
                    # Compilar a ejecutable y ejecutar
                    with st.spinner("Compilando a ejecutable..."):
                        success_exe, message_exe, exe_path = compile_assembly_to_executable(asm_path)
                    
                    if success_exe:
                        st.markdown(f'<div class="success">{message_exe}</div>', unsafe_allow_html=True)
                        
                        # Ejecutar
                        with st.spinner("Ejecutando programa..."):
                            success_run, output, error = run_executable(exe_path)
                        
                        if success_run:
                            st.markdown("### Resultado de la ejecución:")
                            st.markdown('<div class="code-container" style="background-color: #f0f0f0;">', unsafe_allow_html=True)
                            st.text(output)
                            st.markdown('</div>', unsafe_allow_html=True)
                        else:
                            st.markdown(f'<div class="error">Error al ejecutar: {error}</div>', unsafe_allow_html=True)
                    else:
                        st.markdown(f'<div class="error">{message_exe}</div>', unsafe_allow_html=True)
                except Exception as e:
                    st.error(f"Error al procesar el archivo: {str(e)}")
            else:
                st.markdown(f'<div class="error">{message}</div>', unsafe_allow_html=True)
    else:
        st.warning("No se encontraron ejemplos disponibles.")

elif selection == "Preguntas Frecuentes":
    st.markdown('<div class="section-title">Preguntas Frecuentes (FAQ)</div>', unsafe_allow_html=True)
    
    faq = [
        ("¿Qué hace este compilador?", 
         "Este compilador traduce un subconjunto del lenguaje Go a código ensamblador x86_64, permitiendo la ejecución de programas Go simples en arquitecturas compatibles."),
        
        ("¿Qué características de Go están soportadas?", 
         """
         - **Tipos de datos**: enteros (`int`), cadenas (`string`), booleanos (`bool`), estructuras básicas
         - **Expresiones**: operadores aritméticos, lógicos, de comparación y unarios
         - **Sentencias**: declaración de variables, asignaciones, condicionales, bucles, funciones y retornos
         - **Biblioteca estándar**: soporte básico para `fmt.Println()`
         """),
        
        ("¿Cómo puedo ejecutar el código ensamblador generado?", 
         """
         1. Compila el código ensamblador usando GCC:
            ```
            gcc -no-pie -o programa resultado/result.s
            ```
         2. Ejecuta el programa resultante:
            ```
            ./programa
            ```
         """),
        
        ("¿Qué limitaciones tiene este compilador?", 
         """
         - No se admiten arrays multidimensionales
         - Soporte limitado para punteros y referencias
         - No hay verificación de tipos completa
         - No se implementa recolección de basura
         - Soporte limitado para la biblioteca estándar
         - No se admiten goroutines ni canales
         - No se implementa interfaz completa para estructuras
         """),
        
        ("¿Cuál es el flujo del compilador?", 
         """
         1. **Análisis Léxico**: Divide el código fuente en tokens
         2. **Análisis Sintáctico**: Construye un Árbol de Sintaxis Abstracta (AST)
         3. **Generación de Código**: Traduce el AST a instrucciones ensamblador x86_64
         """),
        
        ("¿Este compilador es para uso en producción?", 
         "No, este compilador fue desarrollado con fines educativos y de investigación en el curso de Compiladores en UTEC.")
    ]
    
    for question, answer in faq:
        with st.expander(question):
            st.markdown(answer)

# Pie de página
st.markdown('<div class="footer">Desarrollado como proyecto del curso de Compiladores en UTEC</div>', unsafe_allow_html=True)
