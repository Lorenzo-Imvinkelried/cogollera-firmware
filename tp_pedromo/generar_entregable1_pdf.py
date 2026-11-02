import os
import sys
from reportlab.lib.pagesizes import A4
from reportlab.lib import colors
from reportlab.lib.units import cm
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, KeepTogether, PageBreak
)
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        print(f"[PAGINACIÓN]: Total de páginas generadas = {num_pages}")
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_decorations(num_pages)
            super(NumberedCanvas, self).showPage()
        super(NumberedCanvas, self).save()

    def draw_page_decorations(self, page_count):
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#4B5563"))
        
        # Header (pages 1 and 2)
        self.setStrokeColor(colors.HexColor("#CBD5E1"))
        self.setLineWidth(0.5)
        self.line(1.8 * cm, 28.3 * cm, 19.2 * cm, 28.3 * cm)
        self.drawString(1.8 * cm, 28.5 * cm, "UTN FRSF — ISI — Tecnologías para la Automatización")
        self.drawRightString(19.2 * cm, 28.5 * cm, "Entregable 1: Propuesta Inicial (04/11/2026)")
        
        # Footer
        self.line(1.8 * cm, 1.4 * cm, 19.2 * cm, 1.4 * cm)
        self.drawString(1.8 * cm, 1.0 * cm, "Prof. Jorge R. Vega — Ayudante: Mariano M. Perdomo | Clasificador de Huevos 3 Carriles")
        self.drawRightString(19.2 * cm, 1.0 * cm, f"Página {self._pageNumber} de {page_count}")
        self.restoreState()

def build_pdf(filename):
    doc = SimpleDocTemplate(
        filename,
        pagesize=A4,
        leftMargin=1.8 * cm,
        rightMargin=1.8 * cm,
        topMargin=1.8 * cm,
        bottomMargin=1.8 * cm
    )

    styles = getSampleStyleSheet()

    # Custom styles
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=15,
        leading=18,
        textColor=colors.HexColor("#1E3A8A"), # Dark Navy Blue
        spaceAfter=3
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=10,
        leading=13,
        textColor=colors.HexColor("#0D9488"), # Teal
        spaceAfter=10
    )

    h1_style = ParagraphStyle(
        'SectionH1',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=11,
        leading=14,
        textColor=colors.HexColor("#1E3A8A"),
        spaceBefore=7,
        spaceAfter=3,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'SectionH2',
        parent=styles['Heading3'],
        fontName='Helvetica-Bold',
        fontSize=9.5,
        leading=12,
        textColor=colors.HexColor("#1F2937"),
        spaceBefore=4,
        spaceAfter=2,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'DocBody',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.5,
        leading=11.5,
        textColor=colors.HexColor("#1F2937"),
        alignment=4, # Justified
        spaceAfter=4
    )

    body_bold = ParagraphStyle(
        'DocBodyBold',
        parent=body_style,
        fontName='Helvetica-Bold'
    )

    bullet_style = ParagraphStyle(
        'DocBullet',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.3,
        leading=11,
        textColor=colors.HexColor("#1F2937"),
        leftIndent=12,
        firstLineIndent=-8,
        spaceAfter=2
    )

    meta_table_style = ParagraphStyle(
        'MetaText',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=10.5,
        textColor=colors.HexColor("#374151")
    )

    story = []

    # Title & Metadata Card
    story.append(Paragraph("PROYECTO: CLASIFICADOR AUTOMÁTICO DE HUEVOS EN 3 CARRILES", title_style))
    story.append(Paragraph("Entregable 1 — Propuesta Inicial de Solución con SoC ESP32", subtitle_style))

    meta_data = [
        [
            Paragraph("<b>Institución:</b> UTN Facultad Regional Santa Fe", meta_table_style),
            Paragraph("<b>Asignatura:</b> Tecnologías para la Automatización", meta_table_style)
        ],
        [
            Paragraph("<b>Carrera:</b> Ingeniería en Sistemas de Información", meta_table_style),
            Paragraph("<b>Docentes:</b> Prof. Jorge R. Vega — Ayudante: Mariano M. Perdomo", meta_table_style)
        ],
        [
            Paragraph("<b>Fecha Límite:</b> 04/11/2026", meta_table_style),
            Paragraph("<b>Entorno de Simulación:</b> Wokwi ESP32 (Arquitectura C++ / POO)", meta_table_style)
        ]
    ]
    t_meta = Table(meta_data, colWidths=[8.5 * cm, 8.9 * cm])
    t_meta.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#E2E8F0")),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor("#F1F5F9")),
        ('TOPPADDING', (0,0), (-1,-1), 3),
        ('BOTTOMPADDING', (0,0), (-1,-1), 3),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
    ]))
    story.append(t_meta)
    story.append(Spacer(1, 6))

    # 1. Descripción del problema
    story.append(Paragraph("1. Descripción del Problema a Resolver", h1_style))
    story.append(Paragraph(
        "En la industria avícola de la región centro del país (Santa Fe y Entre Ríos), la selección y envasado "
        "de huevos de consumo según su calibre comercial (Chico, Mediano y Grande) constituye una etapa crítica de "
        "la cadena productiva. En establecimientos medianos y pequeños, esta tarea suele ejecutarse de forma "
        "artesanal o manual mediante balanzas estáticas o simple estimación visual del operario. "
        "Esta modalidad presenta serias desventajas operativas: <b>baja tasa de procesamiento</b>, "
        "<b>elevado margen de error humano en la categorización</b>, <b>riesgo permanente de fisuras o roturas "
        "por manipulación excesiva</b> y una <b>ausencia total de registro digital en tiempo real</b> sobre "
        "el rendimiento por lote.",
        body_style
    ))
    story.append(Paragraph(
        "<b>Problema no trivial planteado:</b> Se requiere una estación automatizada que coordine en tiempo real "
        "el transporte indexado y sincronizado de cada unidad, la medición dimensional acústica sin contacto físico "
        "(para preservar la integridad mecánica de la cáscara), la toma de decisión autónoma mediante lógica "
        "clasificatoria, el redireccionamiento electromecánico suave a tres carriles de descarga independientes y la "
        "protección del puesto de trabajo mediante parada de emergencia asíncrona.",
        body_style
    ))

    # 2. Justificación
    story.append(Paragraph("2. Justificación: Por qué tiene sentido abordarlo con ESP32", h1_style))
    story.append(Paragraph(
        "El microcontrolador ESP32 (SoC de Espressif Systems con arquitectura dual-core Xtensa a 240 MHz) resulta "
        "la plataforma idónea para resolver esta problemática por las siguientes razones técnicas:",
        body_style
    ))
    story.append(Paragraph(
        "• <b>Respuesta temporal determinista y sin jitter:</b> El avance indexado de la cinta transportadora requiere "
        "un tren de pulsos con tiempos precisos de microsegundos (controlador A4988), mientras que el servomotor de "
        "desvío opera sobre modulación PWM precisa a 50 Hz. Los temporizadores de hardware del ESP32 garantizan "
        "ambas modulaciones sin degradar el tiempo de cálculo de la CPU.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Seguridad industrial en tiempo real (Interrupciones externas en IRAM):</b> Ante un atasco o peligro para "
        "el operador, la rutina de interrupción (ISR) se ejecuta en nanosegundos directamente desde memoria RAM "
        "(<code>IRAM_ATTR</code>), inhibiendo instantáneamente los pulsos de potencia independientemente del estado "
        "secuencial del sistema.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Periféricos de conversión analógica (ADC1):</b> Su conversor ADC de 12 bits permite una resolución de 4096 "
        "niveles, ofreciendo una regulación analógica suave y precisa para el potenciómetro de velocidad de línea sin "
        "interferir con las tareas lógicas.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Potencial de conectividad Industria 4.0:</b> Aunque la propuesta inicial se centra en la lógica de control "
        "y simulación mecatrónica en Wokwi, el ESP32 cuenta con conectividad Wi-Fi/BLE integrada. Esto habilita una "
        "extensión natural hacia el reporte de telemetría en tiempo real (Supabase, MQTT o dashboards web de planta) sin "
        "requerir módulos de red complementarios.",
        bullet_style
    ))

    story.append(Spacer(1, 4))
    story.append(PageBreak())

    # 3. Solución Propuesta Resumida
    story.append(Paragraph("3. Solución Propuesta Resumida: Componentes y Lógica General", h1_style))
    
    # 3.1 Cumplimiento de condiciones mínimas (Tabla)
    story.append(Paragraph("3.1 Matriz de Integración de Componentes (Cumplimiento de Condiciones Mínimas)", h2_style))
    
    table_data = [
        [
            Paragraph("<b>Requisito Obligatorio</b>", meta_table_style),
            Paragraph("<b>Componente Implementado</b>", meta_table_style),
            Paragraph("<b>Pin ESP32</b>", meta_table_style),
            Paragraph("<b>Función en el Sistema</b>", meta_table_style)
        ],
        [
            Paragraph("<b>Entrada Digital</b>", meta_table_style),
            Paragraph("Pulsador tipo Hongo E-Stop<br/>(Modo INPUT_PULLUP)", meta_table_style),
            Paragraph("GPIO 26<br/>(Interrupción)", meta_table_style),
            Paragraph("Parada de emergencia por hardware asíncrona ante atascos o riesgos.", meta_table_style)
        ],
        [
            Paragraph("<b>Salida Digital</b>", meta_table_style),
            Paragraph("Señales DIR y STEP del Driver A4988<br/>+ Disparo HC-SR04", meta_table_style),
            Paragraph("GPIO 14 (DIR)<br/>GPIO 12 (STEP)<br/>GPIO 5 (TRIG)", meta_table_style),
            Paragraph("Control de dirección y avance de pasos del motor de cinta + tren acústico.", meta_table_style)
        ],
        [
            Paragraph("<b>Entrada Analógica</b>", meta_table_style),
            Paragraph("Potenciómetro Lineal 10 kΩ<br/>(Conversor ADC de 12 bits)", meta_table_style),
            Paragraph("GPIO 35<br/>(ADC1_CH7)", meta_table_style),
            Paragraph("Regulación en vivo de la velocidad de traslación de la cinta (delay entre pasos).", meta_table_style)
        ],
        [
            Paragraph("<b>Salida PWM</b>", meta_table_style),
            Paragraph("Servomotor de 50 Hz<br/>(Biblioteca ESP32Servo)", meta_table_style),
            Paragraph("GPIO 33<br/>(Timer LEDC)", meta_table_style),
            Paragraph("Control angular proporcional para posicionar la compuerta desviadora tridireccional.", meta_table_style)
        ],
        [
            Paragraph("<b>Sensor</b>", meta_table_style),
            Paragraph("Sensor Ultrasónico HC-SR04<br/>(Tiempo de vuelo acústico)", meta_table_style),
            Paragraph("GPIO 5 (TRIG)<br/>GPIO 18 (ECHO)", meta_table_style),
            Paragraph("Medición sin contacto de la altura de cada huevo: Calibre = 12 cm - Distancia.", meta_table_style)
        ],
        [
            Paragraph("<b>Actuador</b>", meta_table_style),
            Paragraph("Motor Paso a Paso Bipolar (NEMA 17)<br/>+ Servomotor SG90 / MG995", meta_table_style),
            Paragraph("Driver A4988<br/>+ GPIO 33", meta_table_style),
            Paragraph("Accionamiento cinemático de cinta indexada y desvío mecánico a 3 carriles.", meta_table_style)
        ],
        [
            Paragraph("<b>Interacción Usuario</b>", meta_table_style),
            Paragraph("Potenciómetro + Botón E-Stop<br/>+ Telemetría Monitor Serie UART", meta_table_style),
            Paragraph("GPIO 35 / 26<br/>UART 115200", meta_table_style),
            Paragraph("Ingreso analógico de consignas + visualización continua de métricas por lote.", meta_table_style)
        ]
    ]

    t_comp = Table(table_data, colWidths=[3.2 * cm, 4.4 * cm, 2.7 * cm, 7.1 * cm])
    t_comp.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor("#1E3A8A")),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('ALIGN', (0,0), (-1,-1), 'LEFT'),
        ('VALIGN', (0,0), (-1,-1), 'MIDDLE'),
        ('BOTTOMPADDING', (0,0), (-1,-1), 2),
        ('TOPPADDING', (0,0), (-1,-1), 2),
        ('LEFTPADDING', (0,0), (-1,-1), 4),
        ('RIGHTPADDING', (0,0), (-1,-1), 4),
        ('ROWBACKGROUNDS', (0,1), (-1,-1), [colors.white, colors.HexColor("#F8FAFC")]),
        ('GRID', (0,0), (-1,-1), 0.5, colors.HexColor("#CBD5E1")),
    ]))
    story.append(t_comp)
    story.append(Spacer(1, 4))

    # 3.2 Lógica General y Máquina de Estados
    story.append(Paragraph("3.2 Lógica General del Algoritmo (Máquina de Estados Finitos - FSM)", h2_style))
    story.append(Paragraph(
        "El software se estructura bajo el paradigma de <b>Máquina de Estados Finitos no bloqueante</b> gobernada "
        "por marcas de tiempo (<code>millis()</code> y <code>delayMicroseconds()</code> específicos), garantizando "
        "robustez, modularidad y reactividad ante eventos asíncronos:",
        body_style
    ))

    fsm_steps = [
        "<b>1. Estado AVANZAR_CINTA:</b> El ESP32 genera exactamente 200 pulsos digitales sobre el pin STEP del A4988. "
        "Esto provoca un avance angular de 360° en el rodillo motriz, desplazando la cinta transportadora la distancia justa "
        "para posicionar una nueva unidad de huevo bajo la campana del sensor ultrasónico. La cadencia de pulsos es modulada "
        "dinámicamente según la lectura del potenciómetro.",
        
        "<b>2. Estado MEDIR_CALIBRE:</b> El microcontrolador dispara una ráfaga ultrasónica de 10 µs y captura el tiempo de eco "
        "mediante <code>pulseIn()</code>. Considerando la altura base libre de la cinta (12.0 cm), calcula la cota vertical: "
        "<i>Calibre = 12.0 cm - Distancia_Medida</i>. Según este valor, clasifica el espécimen: "
        "<b>Chico / S</b> (< 4.0 cm), <b>Mediano / M</b> (4.0 a 5.5 cm) o <b>Grande / L</b> (> 5.5 cm). "
        "<i>(Nota: Para la simulación dinámica se incluye un generador pseudo-aleatorio que modela la variabilidad continua del lote)</i>.",
        
        "<b>3. Estado POSICIONAR_DESVIADOR:</b> El servomotor es comandado mediante un algoritmo de transición suave grado a grado "
        "para evitar inercia y rebotes mecánicos, orientando la compuerta tridireccional: <b>0° hacia Carril 1</b> (Chicos), "
        "<b>90° hacia Carril 2</b> (Medianos) o <b>180° hacia Carril 3</b> (Grandes).",
        
        "<b>4. Estado DESCARGA_CARRIL:</b> El motor paso a paso ejecuta un empuje corto auxiliar de 80 pasos para asegurar el "
        "descenso suave del huevo hacia la rampa receptora. Se actualiza la base estadística de producción y el sistema retorna "
        "al Estado 1 para la siguiente unidad.",

        "<b>5. Estado PARADA_EMERGENCIA (Supervisión Concurrente):</b> Si el operador acciona el pulsador físico, la ISR "
        "interrumpe el ciclo en nanosegundos, inhibe los pulsos al driver, congela la posición angular del servo y emite un "
        "mensaje de bloqueo en consola serie hasta que el pulsador sea restablecido manualmente."
    ]

    for step in fsm_steps:
        story.append(Paragraph(f"• {step}", bullet_style))

    story.append(Spacer(1, 4))
    story.append(Paragraph(
        "<b>Viabilidad de Validación:</b> La totalidad del circuito, la cinemática del motor paso a paso, el accionamiento "
        "angular del servo, la respuesta acústica y el tablero de telemetría serie se encuentran actualmente verificados y "
        "operativos en el entorno de simulación <b>Wokwi</b>, listos para la fase de implementación y pruebas del Entregable 2.",
        body_style
    ))

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"[OK] PDF generado exitosamente en: {filename}")

if __name__ == "__main__":
    output_pdf = os.path.join(r"D:\Usuario\Desktop\cogollera-firmware\tp_pedromo", "Entregable_1_Propuesta_Inicial.pdf")
    build_pdf(output_pdf)
