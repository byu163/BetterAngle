import os
from PIL import Image, ImageDraw

def process_logo(src_path, output_png, output_ico):
    if not os.path.exists(src_path):
        print(f"Error: Source image not found at {src_path}")
        return

    # Open image
    img = Image.open(src_path).convert("RGBA")
    w, h = img.size
    
    # Create circular mask
    # The logo in the screenshot is centered and roughly a circle.
    # I'll create a mask that keeps only the circular part.
    mask = Image.new("L", (w, h), 0)
    draw = ImageDraw.Draw(mask)
    
    # Define circular area (adjusting based on typical screenshot padding)
    # Usually the logo takes up the majority of the frame.
    margin = 10
    draw.ellipse((margin, margin, w - margin, h - margin), fill=255)
    
    # Apply mask
    output = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    output.paste(img, (0, 0), mask=mask)
    
    # Crop to content to remove excess transparent padding
    bbox = output.getbbox()
    if bbox:
        output = output.crop(bbox)
    
    # Normalize to square
    size = max(output.size)
    new_img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    new_img.paste(output, ((size - output.width) // 2, (size - output.height) // 2))
    
    # Save PNG
    new_img.save(output_png, "PNG")
    print(f"Generated {output_png}")
    
    # Save ICO with multiple resolutions
    ico_sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    new_img.save(output_ico, format="ICO", sizes=ico_sizes)
    print(f"Generated {output_ico}")

if __name__ == "__main__":
    src = "/Users/kierenpatel/.gemini/antigravity/brain/f53cffab-f06d-4312-bc77-9f7aa62de87e/media__1776093655328.png"
    assets_dir = "/Users/kierenpatel/.gemini/antigravity/scratch/BetterAngle/assets"
    
    process_logo(src, 
                 os.path.join(assets_dir, "logo.png"), 
                 os.path.join(assets_dir, "icon.ico"))
