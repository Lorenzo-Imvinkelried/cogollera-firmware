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
        self.drawString(1.8 * cm, 1.0 * cm, "Prof. Jorge R. Vega — Ayudante: Mariano M. Perdomo | Clasificador de Huevos con LCD I2C")
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
        fontSize=14,
        leading=17,
        textColor=colors.HexColor("#1E3A8A"), # Dark Navy Blue
        spaceAfter=2
    )

    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=9.5,
        leading=12,
        textColor=colors.HexColor("#0D9488"), # Teal
        spaceAfter=8
    )

    h1_style = ParagraphStyle(
        'SectionH1',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=13.5,
        textColor=colors.HexColor("#1E3A8A"),
        spaceBefore=6,
        spaceAfter=3,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'SectionH2',
        parent=styles['Heading3'],
        fontName='Helvetica-Bold',
        fontSize=9,
        leading=11.5,
        textColor=colors.HexColor("#1F2937"),
        spaceBefore=4,
        spaceAfter=2,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'DocBody',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8.3,
        leading=11,
        textColor=colors.HexColor("#1F2937"),
        alignment=4, # Justified
        spaceAfter=3
    )

    bullet_style = ParagraphStyle(
        'DocBullet',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=10.5,
        textColor=colors.HexColor("#1F2937"),
        leftIndent=11,
        firstLineIndent=-7,
        spaceAfter=2
    )

    meta_table_style = ParagraphStyle(
        'MetaText',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=7.8,
        leading=10,
        textColor=colors.HexColor("#374151")
    )

    story = []

    # Title & Metadata Card
    story.append(Paragraph("PROYECTO: CLASIFICADOR AUTOMÁTICO DE HUEVOS EN 3 CARRILES", title_style))
    story.append(Paragraph("Entregable 1 — Propuesta Inicial con Pantalla LCD 16x2 y Control de Stepper", subtitle_style))

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
            Paragraph("<b>Simulación:</b> Wokwi ESP32 (LCD I2C, A4988, Servo, HC-SR04)", meta_table_style)
        ]
    ]
    t_meta = Table(meta_data, colWidths=[8.5 * cm, 8.9 * cm])
    t_meta.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor("#F8FAFC")),
        ('BOX', (0,0), (-1,-1), 0.5, colors.HexColor("#E2E8F0")),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor("#F1F5F9")),
        ('TOPPADDING', (0,0), (-1,-1), 2),
        ('BOTTOMPADDING', (0,0), (-1,-1), 2),
        ('LEFTPADDING', (0,0), (-1,-1), 5),
        ('RIGHTPADDING', (0,0), (-1,-1), 5),
    ]))
    story.append(t_meta)
    story.append(Spacer(1, 4))

    # 1. Descripción del problema
    story.append(Paragraph("1. Descripción del Problema a Resolver", h1_style))
    story.append(Paragraph(
        "En la industria avícola de la región centro del país (Santa Fe y Entre Ríos), la selección y envasado "
        "de huevos de consumo según su calibre comercial (Chico, Mediano y Grande) constituye una etapa crítica de "
        "la cadena productiva. En establecimientos medianos y pequeños, esta tarea suele ejecutarse de forma "
        "artesanal mediante estimación visual del operario. "
        "Esta modalidad presenta serias desventajas operativas: <b>baja cadencia de procesamiento</b>, "
        "<b>errores recurrentes en la categorización de calibres</b>, <b>riesgo de roturas por manipulación excesiva</b> "
        "y <b>falta de visibilidad directa de las métricas de producción acumuladas</b>.",
        body_style
    ))
    story.append(Paragraph(
        "<b>Problema no trivial planteado:</b> Se requiere desarrollar una estación automatizada que coordine en tiempo "
        "real el transporte indexado de las unidades mediante un motor paso a paso, la regulación interactiva de la velocidad "
        "de traslación mediante un potenciómetro, la medición acústica de calibre sin contacto (sensor ultrasónico HC-SR04), "
        "la derivación electromecánica suave a 3 carriles de empaque (servomotor) y la visualización instantánea y continua "
        "de los contadores totales y por categoría sobre una <b>pantalla industrial LCD 16x2 comunicada por bus I2C</b>, "
        "integrando además una parada de emergencia asíncrona para seguridad operativa.",
        body_style
    ))

    # 2. Justificación
    story.append(Paragraph("2. Justificación: Por qué tiene sentido abordarlo con ESP32", h1_style))
    story.append(Paragraph(
        "El microcontrolador ESP32 (SoC Xtensa Dual-Core a 240 MHz) resulta la plataforma ideal por sus recursos de hardware:",
        body_style
    ))
    story.append(Paragraph(
        "• <b>Control determinista del motor paso a paso:</b> El accionamiento del driver A4988 demanda trenes de pulsos precisos "
        "en microsegundos. El ESP32 permite regular este retardo sin jitter a partir de la lectura del conversor ADC de 12 bits "
        "del potenciómetro, reflejando el cambio de velocidad instantáneamente en la pantalla LCD.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Interfaz de comunicación I2C por hardware:</b> La integración de la pantalla LCD 16x2 mediante el bus I2C "
        "(pines dedicados GPIO 21 SDA y GPIO 22 SCL) permite comandar el display utilizando únicamente dos líneas digitales, "
        "ahorrando pines GPIO y evitando bloqueos gracias al hardware I2C nativo del ESP32.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Modulación PWM para el servomotor:</b> Los temporizadores por hardware del módulo LEDC generan una señal PWM "
        "limpia a 50 Hz para el posicionamiento exacto y fluido del brazo desviador tridireccional.",
        bullet_style
    ))
    story.append(Paragraph(
        "• <b>Seguridad en tiempo real (Interrupciones externas en IRAM):</b> Ante atascos mecánicos, la parada de emergencia "
        "se ejecuta mediante una ISR en memoria RAM (<code>IRAM_ATTR</code>), congelando el motor stepper de inmediato y "
        "emitiendo la alerta en la pantalla LCD sin depender del ciclo secuencial del programa.",
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
            Paragraph("Parada de emergencia por hardware asíncrona; congela el motor y alerta en LCD.", meta_table_style)
        ],
        [
            Paragraph("<b>Salida Digital</b>", meta_table_style),
            Paragraph("Driver A4988 (DIR / STEP)<br/>+ Disparo Sensor HC-SR04", meta_table_style),
            Paragraph("GPIO 14 (DIR)<br/>GPIO 12 (STEP)<br/>GPIO 5 (TRIG)", meta_table_style),
            Paragraph("Control cinemático de pasos de la cinta + emisión del tren de pulsos ultrasónicos.", meta_table_style)
        ],
        [
            Paragraph("<b>Entrada Analógica</b>", meta_table_style),
            Paragraph("Potenciómetro Lineal 10 kΩ<br/>(ADC de 12 bits: 0 a 4095)", meta_table_style),
            Paragraph("GPIO 35<br/>(ADC1_CH7)", meta_table_style),
            Paragraph("Regulación interactiva de la velocidad del stepper (20% a 100%), visible en LCD.", meta_table_style)
        ],
        [
            Paragraph("<b>Salida PWM</b>", meta_table_style),
            Paragraph("Servomotor angular a 50 Hz<br/>(Biblioteca ESP32Servo)", meta_table_style),
            Paragraph("GPIO 33<br/>(Timer LEDC)", meta_table_style),
            Paragraph("Posicionamiento suave de compuerta a 0° (Chico), 90° (Mediano) o 180° (Grande).", meta_table_style)
        ],
        [
            Paragraph("<b>Sensor</b>", meta_table_style),
            Paragraph("Sensor Ultrasónico HC-SR04<br/>(Tiempo de vuelo acústico)", meta_table_style),
            Paragraph("GPIO 5 (TRIG)<br/>GPIO 18 (ECHO)", meta_table_style),
            Paragraph("Medición sin contacto de la altura de cada huevo: Calibre = 12 cm - Distancia.", meta_table_style)
        ],
        [
            Paragraph("<b>Actuador</b>", meta_table_style),
            Paragraph("Motor Paso a Paso (NEMA 17)<br/>+ Servomotor SG90 / MG995", meta_table_style),
            Paragraph("Driver A4988<br/>+ GPIO 33", meta_table_style),
            Paragraph("Accionamiento cinemático de cinta transportadora y derivación mecánica a 3 carriles.", meta_table_style)
        ],
        [
            Paragraph("<b>Visualización / HMI</b>", meta_table_style),
            Paragraph("<b>Pantalla LCD 16x2 (I2C)</b><br/>+ Potenciómetro + E-Stop", meta_table_style),
            Paragraph("GPIO 21 (SDA)<br/>GPIO 22 (SCL)", meta_table_style),
            Paragraph("Visualización permanente de: Total procesado, Velocidad del Stepper y contadores S/M/L.", meta_table_style)
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
    story.append(Spacer(1, 3))

    # 3.2 Lógica General y Máquina de Estados
    story.append(Paragraph("3.2 Lógica General del Algoritmo (Máquina de Estados Finitos - FSM)", h2_style))
    story.append(Paragraph(
        "El sistema opera bajo una <b>Máquina de Estados Finitos no bloqueante</b> que sincroniza la cinemática, "
        "la adquisición sensorial y la actualización en vivo de la pantalla LCD:",
        body_style
    ))

    fsm_steps = [
        "<b>1. Estado AVANZAR_CINTA:</b> El ESP32 lee el potenciómetro y calcula la velocidad porcentual y el retardo entre pasos "
        "(3000 µs a 1000 µs), actualizando la pantalla LCD (<i>TOT: XX  V: XX%</i>). Genera 200 pulsos sobre el pin STEP del A4988 "
        "para avanzar un casillero y ubicar el huevo bajo el sensor acústico.",
        
        "<b>2. Estado MEDIR_CALIBRE:</b> Dispara el sensor HC-SR04 y calcula la altura vertical (<i>Calibre = 12.0 cm - Distancia</i>). "
        "Categoriza la unidad en <b>Chico / S</b> (< 4.0 cm), <b>Mediano / M</b> (4.0 a 5.5 cm) o <b>Grande / L</b> (> 5.5 cm). "
        "Muestra temporalmente en la pantalla LCD la categoría detectada (ej: <i>HUEVO MEDIANO(M)</i>) e incrementa el contador respectivo.",
        
        "<b>3. Estado POSICIONAR_DESVIADOR:</b> El servomotor ejecuta un barrido suave grado a grado para orientar la compuerta "
        "hacia el carril correspondiente: <b>0° (Carril 1 - Chicos)</b>, <b>90° (Carril 2 - Medianos)</b> o <b>180° (Carril 3 - Grandes)</b>.",
        
        "<b>4. Estado DESCARGA_CARRIL:</b> El motor stepper ejecuta un avance de 80 pasos para depositar el huevo en la rampa receptora. "
        "La pantalla LCD refresca los totales (<i>C: XX  M: XX  G: XX</i>) y el ciclo retorna al Estado 1.",

        "<b>5. Estado PARADA_EMERGENCIA (Asíncrono por ISR):</b> Si se presiona el botón rojo, la interrupción bloquea los pulsos del "
        "motor de forma inmediata y escribe en la pantalla LCD: <i>!EMERGENCIA E-STOP / CINTA DETENIDA</i> hasta su rearme manual."
    ]

    for step in fsm_steps:
        story.append(Paragraph(f"• {step}", bullet_style))

    story.append(Spacer(1, 3))
    story.append(Paragraph(
        "<b>Estado de Validación:</b> El circuito completo, la lógica de estados, la integración del bus I2C con la pantalla "
        "LCD 16x2 y el accionamiento cinemático se encuentran 100% implementados y verificados en el simulador <b>Wokwi</b>.",
        body_style
    ))

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"[OK] PDF generado exitosamente en: {filename}")

if __name__ == "__main__":
    output_pdf = os.path.join(r"D:\Usuario\Desktop\cogollera-firmware\tp_pedromo", "Entregable_1_Propuesta_Inicial.pdf")
    build_pdf(output_pdf)
