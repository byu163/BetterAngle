from PIL import Image, ImageDraw
import os

def make_transparent_and_ico(source_path, png_path, ico_path):
    # Load source
    img = Image.open(source_path).convert("RGBA")
    width, height = img.size
    
    # Create circular mask
    mask = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(mask)
    # Give a tiny buffer to avoid edge aliases (1% border)
    margin = int(width * 0.01)
    draw.ellipse((margin, margin, width - margin, height - margin), fill=255)
    
    # Apply mask
    result = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    result.paste(img, (0, 0), mask=mask)
    
    # Save PNG
    result.save(png_path, "PNG")
    print(f"Saved transparent PNG to {png_path}")
    
    # Standard Windows ICO sizes with full alpha channel
    # 256 is essential for high DPI, 16-48 for shell/taskbar
    sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    result.save(ico_path, format='ICO', sizes=sizes)
    print(f"Saved multi-resolution ICO to {ico_path}")

if __name__ == "__main__":
    src = "/Users/kierenpatel/.gemini/antigravity/brain/f53cffab-f06d-4312-bc77-9f7aa62de87e/betterangle_logo_transparent_master_1776140951919.png"
    assets_dir = "/Users/kierenpatel/.gemini/antigravity/scratch/BetterAngle/assets"
    
    png_out = os.path.join(assets_dir, "logo.png")
    ico_out = os.path.join(assets_dir, "icon.ico")
    
    make_transparent_and_ico(src, png_out, ico_out)
